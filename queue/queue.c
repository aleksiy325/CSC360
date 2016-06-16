/* Implements the queue abstract data type. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

typedef struct queue_link_data_t *queue_link_t;

/* This structure wraps each element in the queue.
   The queue is implemented as a linked list of these
   structures. */
struct queue_link_data_t {
    queue_element_t e;
    queue_link_t next;
};

/* This structure defines the queue itself.  A queue
   is just a pointer to the first (wrapped) element
   in the queue.  This pointer (head) is NULL if the
   queue is empty. */
struct queue_data_t {
    queue_link_t head;
};

queue_t queue_create() {
    queue_t q = (queue_t) malloc(sizeof(struct queue_data_t));

    if(!q)
        return NULL;            // failed to allocate memory

    q->head = NULL;
    return q;
}

void queue_destroy(queue_t q){
    if(q){
        while(!queue_is_empty(q)){
            queue_link_t oldHead = q->head;
            q->head = oldHead->next;
            free(oldHead);
        }
        free(q);        
    }

}




/* Private */
static queue_link_t queue_new_element(queue_element_t e) {
    queue_link_t qlt = (queue_link_t) malloc(sizeof(struct queue_link_data_t));

    if(!qlt)
        return NULL;            // failed to allocate memory

    qlt->e = e;
    qlt->next = NULL;

    return qlt;
}

void queue_append(queue_t q, queue_element_t e) {
    queue_link_t cur;

    assert(q != NULL);
    
    if(queue_is_empty(q)){  // add first element to empty queue
        q->head = queue_new_element(e);  
        
    }else{
        cur = q->head;
        
        assert(q->head != NULL);
        
        while(cur->next) { 
            cur = cur->next;
        }
        
        cur->next = queue_new_element(e);
    }
}

boolean_t queue_remove(queue_t q, queue_element_t * e) {
    queue_link_t oldHead;

    assert(q != NULL);
    if(queue_is_empty(q))
        return FALSE;

    *e = q->head->e;
    oldHead = q->head;
    q->head = q->head->next;
    free(oldHead);
    
    return TRUE;
}

boolean_t queue_is_empty(queue_t q) {
    assert(q != NULL);
    return (q->head == NULL);
}

void queue_reverse(queue_t q){
    if(!queue_is_empty(q)){
        queue_element_t *e = malloc(sizeof(*e));
        
        queue_remove(q,e);
        queue_reverse(q);
        queue_append(q,*e);
        free(e);
    }
}


void queue_merge(queue_t q, queue_t o, queue_t t, queue_pfcompare_t pf){
    assert(q != NULL);
    
    while(!queue_is_empty(o) && !queue_is_empty(t)){
        
        if((*pf)(o->head->e,t->head->e) == -1){                       // eo < et
            queue_element_t eo;
        
            queue_remove(o, &eo);
            queue_append(q, eo);
        }else{                                                      // eo == et or eo > et
            queue_element_t et; 
            
            queue_remove(t, &et);
            queue_append(q, et);
        }
    }
    
    while(!queue_is_empty(o)){
        queue_element_t eo;
        
        queue_remove(o, &eo); 
        queue_append(q, eo);
    }
    
    while(!queue_is_empty(t)){
        queue_element_t et;
            
        queue_remove(t, &et);
        queue_append(q, et);
    }
    
    assert(queue_is_empty(o) && queue_is_empty(t));
} 

void queue_sort(queue_t q, queue_pfcompare_t pf){
    assert(q != NULL);

    if(!queue_is_empty(q) && q->head->next != NULL){ //if not empty and not 1 element 
               
        queue_element_t e;
        queue_t o = queue_create();
        queue_t t = queue_create();
        boolean_t b = FALSE;
        
        while(!queue_is_empty(q)){
            queue_remove(q, &e);
        
            if(b){
                queue_append(o, e);
            }else{
                queue_append(t, e);
            }
            
            b = !b;             //2 b | ! 2 b
        }
        
        queue_sort(o, pf);
        queue_sort(t, pf);
        queue_merge(q, o, t, pf);
        
        queue_destroy(o);
        queue_destroy(t);
    }
}



/* private */
static boolean_t queue_count_one(queue_element_t e,
                                 queue_pfapply_closure_t cl) {
    int *x = (int *)cl;
    *x = *x + 1;
    return TRUE;
}

int queue_size(queue_t q) {
    int sz = 0;

    queue_apply(q, queue_count_one, &sz);
    return sz;
}

boolean_t queue_apply(queue_t q, queue_pfapply_t pf,
                      queue_pfapply_closure_t cl) {
    assert(q != NULL && pf != NULL);

    if(queue_is_empty(q))
        return FALSE;

    queue_link_t cur;

    for(cur = q->head; cur; cur = cur->next) {
        if(!pf(cur->e, cl))
            break;
    }

    return TRUE;
}
