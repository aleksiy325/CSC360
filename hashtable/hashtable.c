#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define DEFAULT_SIZE 16
#define LOADFACTOR 0.75

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif // FALSE

struct htable_ele_data;

typedef struct htable_ele_data* p_htable_ele_t;

typedef struct htable_ele_data {
    void* val;
    char* key;
    p_htable_ele_t next;
}htable_ele_data_t;


typedef struct htable_data{
    unsigned size;
    unsigned eles;
    htable_hashfunc_t hfunc;
    htable_equalsfunc_t efunc;
    htable_ele_data_t* table;
}htable_data_t;


/* Default functions*/

unsigned jenkins_hash(char *key){
    //JENKINS HASH FUNCTION FROM https://en.wikipedia.org/wiki/Jenkins_hash_function
    int len = strlen(key);
    int i;
    unsigned hash;
    
    for(hash = i = 0; i < len; ++i){
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    
    return hash;
} 

int key_equals(char* str1, char* str2){
    return(strcmp(str1, str2) == 0);
}   

/* Privates */

static unsigned new_count = 0;  //@@@
static unsigned del_count = 0;
static unsigned rem_count = 0; //@@@

p_htable_ele_t htable_new_element(char* key, void* val, p_htable_ele_t next){
 
    p_htable_ele_t e = (p_htable_ele_t) malloc(sizeof(htable_ele_data_t));
    
    if(!e){
        return NULL;
    }
    
    e->val = val;
    e->key = key;
    e->next = next;
    
    new_count++; //@@@
    
    return e;    
}

void htable_resize(htable_t ht, int nsize){
    htable_ele_data_t* oldtable = ht->table;
    int oldsize = ht->size;
    
    ht->table = calloc(nsize, sizeof(*(ht->table)));
    ht->size = nsize;
    ht->eles = 0;
    
    int i = 0;
    for(i; i < oldsize; i++){
        if(oldtable[i].key != NULL){                                   //slot is non empty
            
            htable_add(ht, oldtable[i].key, oldtable[i].val);       //add old element
            
            p_htable_ele_t cur = oldtable[i].next;
            p_htable_ele_t temp;
            while(cur){                                                      // loop through chain
                htable_add(ht, cur->key, cur->val);
                temp = cur;
                cur = cur->next;
                free(temp);
            }
        }
    }
    
    
    free(oldtable);
}

/* Hash Table functions*/

htable_t htable_create(){
    /* allocate space for table */
    htable_t ht = (htable_t) calloc(1, sizeof(htable_data_t) );
    ht->table = (htable_ele_data_t*) calloc(DEFAULT_SIZE, sizeof(htable_ele_data_t));
    
    if(ht){
      
        ht->size = DEFAULT_SIZE;                                    //default initial size
        ht->eles = 0;
        
        /*default functions */
        ht->hfunc = jenkins_hash;
        ht->efunc = key_equals;
        
    }
    
    return ht;
}


void htable_destroy(htable_t ht){
    printf("New counter: %d \n", new_count); //@@@
    if(ht){
        int i = 0;
        
        for(i; i < ht->size; i++){  
            p_htable_ele_t cur = ht->table[i].next;
            while(cur){                                     //loop through list and free
                p_htable_ele_t next = cur->next;
                free(cur);
                del_count++;
                cur = next; //@@@
            }
            
               
        }
        free(ht->table);
        free(ht); 
    } 
    printf("Del counter: %d \n", del_count); //@@@
    printf("Rem counter: %d \n", rem_count); //@@@
}


boolean_t htable_add(htable_t ht, char* key, void* val){
    assert( ht != NULL);
    boolean_t ret = FALSE;
    
    
    if(!htable_ispresent(ht, key)){                                       //duplicate key not allowed
    
        htable_hashfunc_t hf = (ht->hfunc);
        htable_equalsfunc_t ef = (ht->efunc);
   
   
        if(1.0 * ht->eles / ht->size > LOADFACTOR){                     //Resize table
            htable_resize(ht, ht->size * 2);
        }
        
        unsigned k = ((*hf)(key) % ht->size);                           //calc key index
        assert(k < ht->size && k >= 0);
        
       
        if(k < ht->size && k >= 0){
            ret = ht->table[k].key == NULL;
            
            if(ret){                                                    //empty slot
                ht->table[k].key = key;
                ht->table[k].val = val;
                ht->table[k].next = NULL;
                
            }else{                                                      // chain
               
                p_htable_ele_t new = htable_new_element(key, val, ht->table[k].next);
                
                ret = new != NULL;
                
                if(ret){
                    ht->table[k].next = new;        
                } 
            }   
        }
        
        if(ret){
            ht->eles++;
        }
        
    }

    return ret;
}
    
void* htable_lookup(htable_t ht, char* key){
    assert(ht != NULL);
    htable_hashfunc_t hf = (ht->hfunc);
    htable_equalsfunc_t ef = (ht->efunc);
    void* ret  = NULL;
    
    unsigned k = ((*hf)(key) % ht->size);                           //calc key index
    assert(k < ht->size && k >= 0);
    
    if(ht->table[k].key != NULL){                                   //slot is non empty
        if((*ef)(key, ht->table[k].key)){                      //key in first slot
            ret = ht->table[k].val;    
        }else{                                                      //loop through chain    
            p_htable_ele_t cur = ht->table[k].next;
            
            while(cur && !(*ef)(key, cur->key)){
                cur = cur->next;
            }
            if(cur){
                ret = cur->val;
            }
        }
    }
        
    return ret;
}
    
    
boolean_t htable_ispresent(htable_t ht, char* key){
    //same as htable_lookup except rets is boolean_t
    assert(ht != NULL);
    htable_hashfunc_t hf = (ht->hfunc);
    htable_equalsfunc_t ef = (ht->efunc);
    boolean_t ret = FALSE;
    
    unsigned k = ((*hf)(key) % ht->size);                           //calc key index
    assert(k < ht->size && k >= 0);
    
    if(ht->table[k].key != NULL){                                   // slot is not empty
        if((*ef)(key, ht->table[k].key)){                           //key in first slot
            ret = ht->table[k].val;    
        }else{                                                      //loop through chain    
            p_htable_ele_t cur = ht->table[k].next;
            
            while(cur && !(*ef)(key, cur->key)){
                cur = cur->next;
            }
            if(cur){
                ret = cur->val;
            }
        }
    }
    return ret;


}

boolean_t htable_remove(htable_t ht, char* key){
    assert(ht != NULL);
    htable_hashfunc_t hf = (ht->hfunc);
    htable_equalsfunc_t ef = (ht->efunc);
    boolean_t ret = FALSE;
    
    unsigned k = ((*hf)(key) % ht->size);                               //calc key index
    assert(k < ht->size && k >= 0);
   
    if(ht->table[k].key != NULL){                                       // slot is not empty
  
        if((*ef)(key, ht->table[k].key)){                               //key in first slot
            
            if(ht->table[k].next){ 
                p_htable_ele_t next = ht->table[k].next;
                ht->table[k] = *(next);
                free(next);
                rem_count++; //@@@

            }else{
                memset(&(ht->table[k]), 0, sizeof(ht->table[k]));       // overwrite with null or next
            }
            
            ret = TRUE;
        }else{
      
            p_htable_ele_t prev = &(ht->table[k]);
            p_htable_ele_t cur = ht->table[k].next;
            
            while(cur && !(*ef)(key, cur->key)){
                prev = cur;
                cur = cur->next;
                
            }
            if(cur){                                                     //element is present
                prev->next = cur->next;
                free(cur);
                ret = TRUE;
                rem_count++; //@@@
            }
        
        }
    }

    assert( !htable_ispresent(ht, key));                                 //can be removed
    
    if(ret){
        ht->eles--;
    }
    
    return ret;
}


unsigned htable_num_eles(htable_t ht){
    return ht->eles;
}

void htable_set_hash(htable_t ht, htable_hashfunc_t hf){
    ht->hfunc = hf;
}

void htable_set_equals(htable_t ht, htable_equalsfunc_t ef){
    ht->efunc = ef;
}



