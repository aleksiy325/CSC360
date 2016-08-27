/* Simplethreads Instructional Thread Package
 * 
 * sthread_user.c - Implements the sthread API using user-level threads.
 *
 *    You need to implement the routines in this file.
 *
 * Change Log:
 * 2002-04-15        rick
 *   - Initial version.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sthread.h>
#include <sthread_queue.h>
#include <sthread_user.h>
#include <sthread_ctx.h>
#include <sthread_user.h>
#include <sthread_preempt.h>



/*There is still a bug with preemption */ 

struct _sthread {
    int tid;
	sthread_ctx_t *saved_ctx;
    sthread_start_func_t start_routine;
    void* arg;
    void* ret;
	int joinable;
    int complete;
};

int tid;
sthread_queue_t thread_queue;
sthread_queue_t dead_queue;
struct _sthread* cur_thread;

/*********************************************************************/
/* Part 1: Creating and Scheduling Threads                           */
/*********************************************************************/
static int intcount = 1;
static int interrupt = 0;


void enable_int(){
    //assert(intcount > 0);
    //printf("Set LOW\n");
    intcount--;
    interrupt = 0;
    splx(LOW);
}

void disable_int(){
    splx(HIGH);
    //printf("Set HIGH\n");
    interrupt = 1;
    //assert(intcount == 0);
    intcount++;
}

void sthread_aux_start(void){
    enable_int();
    //printf("%d \n", cur_thread);
    cur_thread->ret = cur_thread->start_routine(cur_thread->arg);
    sthread_user_exit((void*)0);
}



void sthread_user_init(void) {
    
    tid = 1;
    thread_queue = sthread_new_queue();
    dead_queue = sthread_new_queue();
    
    //main thread
    
    struct _sthread* main_thread = malloc(sizeof(*main_thread));
    
    main_thread->tid = tid++;
    main_thread->saved_ctx = sthread_new_blank_ctx();
    main_thread->start_routine = NULL;
    main_thread->arg = NULL;
    main_thread->ret = NULL;
    main_thread->joinable = 0;
    main_thread->complete = 0;
 
    cur_thread = main_thread;
    

    sthread_preemption_init(sthread_user_yield, 5000);
    
}

sthread_t sthread_user_create(sthread_start_func_t start_routine, void *arg, int joinable) {
    disable_int();
    sthread_ctx_start_func_t func = sthread_aux_start; 
    
    struct _sthread* new_thread = malloc(sizeof(*new_thread));
    
    new_thread->tid = tid++;
    new_thread->start_routine = start_routine;
    new_thread->arg = arg;
    new_thread->ret = NULL;
    new_thread->joinable = joinable;
    new_thread->complete = 0;
    new_thread->saved_ctx = sthread_new_ctx(func);
    
    sthread_enqueue(thread_queue, new_thread);
    enable_int();
 
    sthread_user_yield();
    
    return new_thread;
}

void sthread_user_exit(void *ret) {
    disable_int();
     
    //Main thread always in queue?
    struct _sthread* old_thread;
    old_thread = cur_thread;
    cur_thread = sthread_dequeue(thread_queue);
    
    
    //joining
    if(old_thread->joinable){
        old_thread->complete = 1;
        
    }else{
        //problem freeing dead threads
        //sthread_user_free(old_thread);
       sthread_enqueue(dead_queue, old_thread); //enque for cleanup
       // sthread_user_free(old_thread);
    }
       
    sthread_switch(old_thread->saved_ctx, cur_thread->saved_ctx);
    
}


void sthread_user_free(struct _sthread *thread)
{
    sthread_free_ctx(thread->saved_ctx);
    free(thread);
}

void* sthread_user_join(sthread_t t) {
    while(!t->complete){}
    disable_int();
    void* ret = t->ret;
    sthread_user_free(t);  // Join multiple
    enable_int();
    
    return ret;
}

void sthread_user_yield(void){
    disable_int();
    //printf("enter yield\n"); fflush(stdout);
    if(!sthread_queue_is_empty(thread_queue)){
        
        struct _sthread* old_thread;
        old_thread = cur_thread;
        cur_thread = sthread_dequeue(thread_queue);
        sthread_enqueue(thread_queue, old_thread);
        
        //printf("Yielding %d, %0x to %d, %0x \n", old_thread->tid, old_thread->saved_ctx->sp,  cur_thread->tid, cur_thread->saved_ctx->sp);
        assert(cur_thread != NULL); 
        //printf("!!!!YIELD Oldthread: %p Newthread %p  Thread %p\n", old_thread, cur_thread, thread_queue);

        sthread_switch(old_thread->saved_ctx, cur_thread->saved_ctx);
 
        
    }else{
        //printf("Queue is empty \n");
    
    }
    //printf("yield low\n");
    enable_int();
   
}


/* Add any new part 1 functions here */


/*********************************************************************/
/* Part 2: Synchronization Primitives                                */
/*********************************************************************/

struct _sthread_mutex {
	lock_t locked;
    struct _sthread* thread;
    
};

sthread_mutex_t sthread_user_mutex_init() {
	struct _sthread_mutex* lock = malloc(sizeof(*lock));
    lock->locked = 0;
    lock->thread = NULL;

    return lock;
}

void sthread_user_mutex_free(sthread_mutex_t lock) {
    free(lock);
}

void sthread_user_mutex_lock(sthread_mutex_t lock) {
    assert(lock != NULL);
    
    while(atomic_test_and_set(&(lock->locked))) {} //loop until free
    
    assert(lock->thread == NULL);
        
    lock->thread = cur_thread;
        
}

void sthread_user_mutex_unlock(sthread_mutex_t lock) {
    assert(lock != NULL);
    assert(lock->thread == cur_thread);
        
    lock->thread = NULL;
    atomic_clear(&(lock->locked));
   
}


struct _sthread_cond {
	lock_t locked;
	sthread_queue_t blocked;
};

sthread_cond_t sthread_user_cond_init(void) {
    sthread_cond_t cond = malloc(sizeof(*cond));
    cond->locked = 0;
    cond->blocked = sthread_new_queue();
    return cond;
}

void sthread_user_cond_free(sthread_cond_t cond) {
    if( cond != NULL){
        sthread_free_queue(cond->blocked);
        free(cond);
    }
}

void sthread_user_cond_signal(sthread_cond_t cond) {
    while(atomic_test_and_set(&(cond->locked))) {} 
    
    if(!sthread_queue_is_empty(cond->blocked)){
        disable_int();
        struct _sthread* old_thread;
        old_thread = sthread_dequeue(cond->blocked);
        sthread_enqueue(thread_queue, old_thread);
        enable_int();
    }
    
    atomic_clear(&(cond->locked));
    
}

void sthread_user_cond_broadcast(sthread_cond_t cond) {
    while(atomic_test_and_set(&(cond->locked))) {} 
    
    while(!sthread_queue_is_empty(cond->blocked)){
        disable_int();
        struct _sthread* old_thread;
        cur_thread = sthread_dequeue(cond->blocked);
        sthread_enqueue(thread_queue, old_thread);
        enable_int();
    }
    
    atomic_clear(&(cond->locked));
}

void sthread_user_cond_wait(sthread_cond_t cond, sthread_mutex_t lock) {
    while(atomic_test_and_set(&(cond->locked))) {} 
    
    sthread_user_mutex_unlock(lock);
    
    struct _sthread* old_thread;
    disable_int();
    old_thread = cur_thread;
    cur_thread = sthread_dequeue(thread_queue);
    sthread_enqueue(cond->blocked, old_thread);
    
    atomic_clear(&(cond->locked));
    
    sthread_switch(old_thread->saved_ctx, cur_thread->saved_ctx);  
    
    sthread_user_mutex_lock(lock);

}


