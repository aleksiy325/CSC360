#define DEBUG 1


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hashtable.h"



int test_hashtable(int n, int max){
    time_t t;   
    srand((unsigned) time(&t));
    char str[n][max];
    int arr[n];
    
    htable_t ht = htable_create();
    
    
    printf("Testing add");
    int i = 0;
    for(i; i < n; i++){         //gen keys and add
     
       sprintf(str[i], "%d", i);
       arr[i] = rand() % (n* 5);
    
       htable_add(ht, str[i], &arr[i]);
       assert(htable_ispresent(ht, str[i]));
    }
    
    //adding duplicate key should fail
    int x = 10;
    //assert(!htable_add(ht, str[--i], &x));
    
    
    for(i=0; i < n; i++){                       //test lookup and remove
       int* y = htable_lookup(ht, str[i]);
       //printf("%d , %d , %s \n", y, i, str[i]);
       assert(arr[i] = *y);
       htable_remove(ht, str[i]);
       assert(!htable_ispresent(ht, str[i]));
    }
    
    printf("All tests passed.");
    
    htable_destroy(ht);

}

int main(int argc, char *argv[]) {    
    test_hashtable(2000, 20);
    return 0;
}