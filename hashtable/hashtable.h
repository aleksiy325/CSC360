#ifndef _HASHTABLE_H
#define _HASHTABLE_H

/*Definitions for abstract hashtable */

struct htable_data;

typedef int boolean_t;
typedef struct htable_data* htable_t;


extern unsigned htable_num_eles(htable_t ht);

/* create and return new hashtable 
else return null*/
extern htable_t htable_create();

/*destroy the table and free up all space */
extern void htable_destroy(htable_t ht);

/*add element to hashtable with a key 
returns true if succeeded*/
extern boolean_t htable_add(htable_t ht, char* key, void* val);

/*remove key and val from hashtable
returns element */
extern boolean_t htable_remove(htable_t ht, char* key);

/* lookup if key is present in htable
return value if found else null */
extern void* htable_lookup(htable_t ht, char* key);

/* lookup if key is present in htable
return true if found else false */
extern boolean_t htable_ispresent(htable_t ht, char* key);


/* set the hash function of the table */
typedef int (*htable_hashfunc_t) (char* key);

extern void htable_set_hash(htable_t ht, htable_hashfunc_t hf);

/* set the equals function testing whether two keys are the same of the table. */
typedef int (*htable_equalsfunc_t) (void* e1, void* e2);

void  htable_set_equals(htable_t ht, htable_equalsfunc_t ef);


#endif //_HASHTABLE_H