/* Defines functions for implmenting a dictionary.
 * CSC 357, Assignment 3
 * Given code, Winter '24 */

#include <stdlib.h>
#include <string.h>
#include "dict.h"


/* dcthash: Hashes a string key.
 * NOTE: This is certainly not the best known string hashing function, but it
 *       is reasonably performant and easy to predict when testing. */
static unsigned long int dcthash(char *key) {
    unsigned long int code = 0, i;

    for (i = 0; i < 8 && key[i] != '\0'; i++) {
        code = key[i] + 31 * code;
    }

    return code;
}


/* dctcreate: Creates a new empty dictionary.
 * TODO: Implement this function. It should return a pointer to a newly
 *       dynamically allocated dictionary with an empty backing array. */
Dict *dctcreate() {
    int i;
    Dict *dict = (Dict *) malloc(sizeof(Dict));
    dict->cap = 5; 

    /* Creating backing array */
    dict->arr = (Node **) malloc(dict->cap * sizeof(Node *));
    for(i = 0; i < dict->cap; ++i) {
        dict->arr[i] = NULL;
    }

    dict-> size = 0; 
    return dict;    
}

/* frees all the nodes in a list given the head */
void lstdestroy(Node *head) {
    Node *next = head; 
    while(head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

/* dctdestroy: Destroys an existing dictionary.
 * TODO: Implement this function. It should deallocate a dictionary, its
 *       backing array, and all of its nodes. */
void dctdestroy(Dict *dct) {
    int i; 
        
    if(!dct) return;

    for(i = 0; i < dct->cap; ++i) {
        lstdestroy(dct->arr[i]);
    }
    free(dct->arr);
    free(dct);

    return;
}

/* dctget: Gets the value to which a key is mapped.
 * TODO: Implement this function. It should return the value to which "key" is
 *       mapped, or NULL if it does not exist. */
void *dctget(Dict *dct, char *key) {
    Node *ptr;
    int index = dcthash(key) % dct->cap;
    ptr = dct->arr[index];
    while(ptr != NULL) {
        if(strcmp(ptr->key, key) == 0) {
            return ptr->val;
        }
        ptr = ptr->next; 
    }

    return NULL;
}

/* dctinsert: Inserts a key, overwriting any existing value.
 * TODO: Implement this function. It should insert "key" mapped to "val".
 * NOTE: Depending on how you use this dictionary, it may be useful to insert a
 *       dynamically allocated copy of "key", rather than "key" itself. Either
 *       implementation is acceptable, as long as there are no memory leaks. */
void dctinsert(Dict *dct, char *key, void *val) {
    
    Node **new_arr;
    int new_cap; 
    int i;

    if(dct->cap <= dct->size) {
        /* allocating the array */
        new_cap = dct->cap * 2 + 1;
        new_arr = (Node **) malloc(new_cap * sizeof(Node*));
        for(i = 0; i < new_cap; ++i) {
            new_arr[i] = NULL;
        }
    
        /* populating new array  */
        { 
            int i; 
            int hash_index;
            Node *curr;
            Node *next; 
            for(i = 0; i < dct->cap; ++i) {
                curr = dct->arr[i];
                while(curr != NULL) {
                    /* Calculate key */
                    hash_index = dcthash(curr->key) % new_cap;
                    /* add node to head */
                    next = curr->next;
                    curr->next = new_arr[hash_index];
                    new_arr[hash_index] = curr;
                    /* increment original list */
                    curr = next;
                }
            }

            /* garbage collect previous array */
            free(dct->arr);

            /* set dictionary to point to the new array */
            dct->cap = new_cap; 
            dct->arr = new_arr;
        }
    }   

    /* inserting new node */
    {
        int index = dcthash(key) % dct->cap; 
        Node *new; 
        Node *curr;
        
        if(dct->arr[index] != NULL) {
            curr = dct->arr[index];
            while(curr != NULL ) {
                if(strcmp(curr->key, key) == 0) {
                    curr->val = val; 
                    return;
                }
                curr = curr->next;
            }
        }

        new = (Node *) malloc(sizeof(Node));
        new->key = key;
        new->val = val;
        new->next = dct->arr[index];
        dct->arr[index] = new;
        dct->size++;
    }
    
    return;
}

/* dctremove: Removes a key and the value to which it is mapped.
 * TODO: Implement this function. It should remove "key" and return the value
 *       to which it was mapped, or NULL if it did not exist. */
void *dctremove(Dict *dct, char *key) {
    int i; 
    Node *curr;
    Node *removed;
    void *val; 

    i = dcthash(key) % dct->cap;
    curr = dct->arr[i];
    if(curr == NULL) {
        return NULL;
    }
    else if(strcmp(curr->key, key) == 0) {
        val = curr->val;
        dct->arr[i] = curr->next;
        free(curr);
        dct->size--;
        return val;
    }
    else {
        while(curr->next != NULL) {
            if(strcmp(curr->next->key, key) == 0) {
                val = curr->next->val;
                removed = curr->next;
                curr->next = curr->next->next;
                free(removed);
                dct->size--;
                return val;
            }
            curr = curr->next;
        }
    }

    return NULL;
}

/* dctkeys: Enumerates all of the keys in a dictionary.
 * TODO: Implement this function. It should return a dynamically allocated array
 *       of pointers to the keys, in no particular order, or NULL if empty. */
char **dctkeys(Dict *dct) {
    int i; 
    int index = 0; 
    Node *curr;
    char **keys;
    if(dct->size == 0) {
        return NULL;
    }

    keys = (char **) malloc(dct->size * sizeof(char*));
    for(i = 0; i < dct->cap; ++i) {
        if(dct->arr[i] != NULL) {
            curr = dct->arr[i];
            while(curr != NULL) {
                keys[index] = curr->key;
                ++index;
                curr = curr->next;
            }
        }
    }
    return keys;
}