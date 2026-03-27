/* Tests functions for implementing a dictionary.
 * CSC 357, Assignment 3
 * Given tests, Winter '24 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dict.h"

/* test01: Tests creating dictionaries. */
void test01() {
    Dict *dct = dctcreate();

    assert(dct != NULL);
    assert(dct->cap >= 1);
    assert(dct->arr != NULL);
    assert(dct->size == 0);

    dctdestroy(dct);
}

/* test02: Tests inserting into dictionaries. */
void test02() {
    Dict *dct = dctcreate();

    dctinsert(dct, "a", (void *)1);
    dctinsert(dct, "b", (void *)3);
    dctinsert(dct, "f", (void *)2);

    assert(dct->size == 3);
    assert(dctget(dct, "a") == (void *)1);
    assert(dctget(dct, "b") == (void *)3);
    assert(dctget(dct, "f") == (void *)2);

    dctdestroy(dct);
}

/* test03: Tests removing from dictionaries. */
void test03() {
    Dict *dct = dctcreate();

    dctinsert(dct, "a", (void *)1);
    dctinsert(dct, "b", (void *)3);
    dctinsert(dct, "f", (void *)2);
    dctremove(dct, "b");
    dctinsert(dct, "c", (void *)2);

    assert(dct->size == 3);
    assert(dctget(dct, "a") == (void *)1);
    assert(dctget(dct, "c") == (void *)2);
    assert(dctget(dct, "f") == (void *)2);
    assert(dctget(dct, "b") == NULL);

    dctdestroy(dct);
}

/* test04: Tests enumerating keys in dictionaries. */
void test04() {
    char **keys;
    Dict *dct = dctcreate();

    dctinsert(dct, "a", (void *)1);
    dctinsert(dct, "b", (void *)3);
    dctinsert(dct, "f", (void *)2);
    dctremove(dct, "b");
    dctinsert(dct, "c", (void *)2);

    /* NOTE: Dictionaries are unordered, and my tests will not assume any
     *       particular arrangement of keys within the enumerating array. */

    keys = dctkeys(dct);
    assert(
     (!strcmp(keys[0], "a") && !strcmp(keys[1], "c") && !strcmp(keys[2], "f"))||
     (!strcmp(keys[0], "a") && !strcmp(keys[1], "f") && !strcmp(keys[2], "c"))||
     (!strcmp(keys[0], "c") && !strcmp(keys[1], "a") && !strcmp(keys[2], "f"))||
     (!strcmp(keys[0], "c") && !strcmp(keys[1], "f") && !strcmp(keys[2], "a"))||
     (!strcmp(keys[0], "f") && !strcmp(keys[1], "a") && !strcmp(keys[2], "c"))||
     (!strcmp(keys[0], "f") && !strcmp(keys[1], "c") && !strcmp(keys[2], "a")));
    free(keys);

    dctdestroy(dct);
}

int main(void) {
    test01();
    test02( );
    test03();
    test04(); 

    return 0;
}
