#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <assert.h>

int x = 0;
int y = 1;
int z = 2;


boolean_t show_one(queue_element_t e, queue_pfapply_closure_t cl) {
    printf("Item %d == %d\n", *(int *)cl, *(int *)e);
    *(int *)cl = *(int *)cl + 1;
    return TRUE;
}

void print_queue(queue_t q){
    printf("\n");
    int index_closure = 0;
    queue_apply(q, show_one, &index_closure);
    printf("\n");
}

int compare_int(int *a , int *b){
    return (*a < *b) ? -1 : (*a > *b);
}


int test_add_remove(int n){
    time_t t;
    queue_t q = queue_create();
    int* arr = malloc(sizeof(*arr)*n);
    int count = 0;
     
    srand((unsigned) time(&t));
    
    
    int i = 0;
    for(; i < n; i++){                             
        int* x = malloc(sizeof(*x));
        *x = rand() % (n*5);
        arr[i] = *x;
        queue_append(q, x);
        assert((i + 1) == queue_size(q));           // test size
    }
    
    for(i = 0; i < n; i++){                         
        int* x = malloc(sizeof(*x));
        queue_remove(q, &x);
        if(arr[i] == *x){
            count++;
        }
        assert((n - i - 1) == queue_size(q)); 
        free(x);
    }
    
    free(arr);
    return(count == n);
}

int test_sort(int n){
    time_t t;
    queue_t q = queue_create();
    int* arr = malloc(sizeof(*arr)*n);
     
    srand((unsigned) time(&t));
    
    printf("Testing sort queue\n");
    
    int i = 0;
    for(; i < n; i++){                              // add random elements in q and array.
        int* x = malloc(sizeof(*x));
        *x = rand() % (n*5);
        arr[i] = *x;
        queue_append(q, x);
    }
    
    print_queue(q);
    queue_sort(q, &compare_int);                    // sort both
        print_queue(q);
    qsort( arr, n, sizeof(*arr), &compare_int);
    
    int count = 0;
    
    for(i = 0; i < n; i++){                         // loop through and compare
        int* x = malloc(sizeof(*x));
        queue_remove(q, &x);
        if(arr[i] == *x){
            count++;
        }
        
        free(x);
    }
    
    free(arr);
    
    return(count == n);
}


int test_reverse(int n){
    time_t t;
    queue_t q = queue_create();
    int* arr = malloc(sizeof(*arr)*n);
    int count = 0;
     
    srand((unsigned) time(&t));
    
    printf("Testing reverse queue\n");
    
    int i = 0;
    for(; i < n; i++){                              // add random elements in q and array.
        int* x = malloc(sizeof(*x));
        *x = rand() % (n*5);
        arr[i] = *x;
        queue_append(q, x);
    }
    
    print_queue(q);
    queue_reverse(q);
    print_queue(q);
    
    for(i = n - 1; i >= 0; i--){                         // loop in reverse
        int* x = malloc(sizeof(*x));
        queue_remove(q, &x);
        if(arr[i] == *x){
            count++;
        }
        
        free(x);
    }
    
    free(arr);
    
    return(count == n);
}

/* A test driver for the queue.  Part of your job is to fill this
   test driver out even more. */
int main(int argc, char *argv[]) {
    queue_t q = queue_create();
    queue_append(q, &x);
    queue_append(q, &y);
    queue_append(q, &x);
    queue_append(q, &z);
    queue_append(q, &y);
    
    queue_reverse(q);
    queue_sort(q, &compare_int);  
    
    queue_destroy(q);
    
    
    return 0;
}
