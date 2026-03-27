/* Tests functions for sorting arrays in parallel.
 * CSC 357, Assignment 7
 * Given tests, Winter '24. */

#include <sys/resource.h>
#include <assert.h>
#include "fsort.h"

int intcmp(const void *a, const void *b) {
    return *((int *)a) - *((int *)b);
}

int chrcmp(const void *a, const void *b) {
    return *((char *)a) - *((char *)b);
}

/* test01: Tests sorting trivial arrays. */
void test01() {
    int arr[] = {1};

    assert(fsort(arr, 0, sizeof(int), 8, intcmp) == 0);
    assert(fsort(arr, 1, sizeof(int), 8, intcmp) == 0);

    assert(arr[0] == 1);
}

/* test02: Tests sorting with trivial processes. */
void test02() {
    int arr[] = {2, -1, 9, 8, 5, -3, 0, 8};

    assert(fsort(arr, 8, sizeof(int), 8, intcmp) == 0);

    assert(arr[0] == -3);
    assert(arr[1] == -1);
    assert(arr[2] == 0);
    assert(arr[3] == 2);
    assert(arr[4] == 5);
    assert(arr[5] == 8);
    assert(arr[6] == 8);
    assert(arr[7] == 9);
}

/* test03: Tests sorting integers. */
void test03() {
    int arr[] = {2, -1, 9, 8, 5, -3, 0, 8};

    assert(fsort(arr, 8, sizeof(int), 4, intcmp) == 0);

    assert(arr[0] == -3);
    assert(arr[1] == -1);
    assert(arr[2] == 0);
    assert(arr[3] == 2);
    assert(arr[4] == 5);
    assert(arr[5] == 8);
    assert(arr[6] == 8);
    assert(arr[7] == 9);
}

/* test04: Tests sorting characters. */
void test04() {
    char arr[] = {'a', 'c', 'b', 'a', 'b', 'c', 'a', 'c'};

    assert(fsort(arr, 8, sizeof(char), 4, chrcmp) == 0);

    assert(arr[0] == 'a');
    assert(arr[1] == 'a');
    assert(arr[2] == 'a');
    assert(arr[3] == 'b');
    assert(arr[4] == 'b');
    assert(arr[5] == 'c');
    assert(arr[6] == 'c');
    assert(arr[7] == 'c');
}

/* test05: Tests sorting with failing forks. */
void test05() {
    int arr[] = {2, -1, 9, 8, 5, -3, 0, 8};
    struct rlimit limit;

    getrlimit(RLIMIT_NPROC, &limit);
    limit.rlim_cur = 1;
    setrlimit(RLIMIT_NPROC, &limit);

    /* NOTE: With the soft limit on number of processes lowered to 1, the first
     *       call to fork will immediately fail... */
    assert(fsort(arr, 8, sizeof(int), 4, intcmp) == 1);

    assert(arr[0] == 2);
    assert(arr[1] == -1);
    assert(arr[2] == 9);
    assert(arr[3] == 8);
    assert(arr[4] == 5);
    assert(arr[5] == -3);
    assert(arr[6] == 0);
    assert(arr[7] == 8);

    /* NOTE: ...however, as long as no calls to fork are ever made, the sort
     *       should succeed. */
    assert(fsort(arr, 8, sizeof(int), 8, intcmp) == 0);

    assert(arr[0] == -3);
    assert(arr[1] == -1);
    assert(arr[2] == 0);
    assert(arr[3] == 2);
    assert(arr[4] == 5);
    assert(arr[5] == 8);
    assert(arr[6] == 8);
    assert(arr[7] == 9);

    getrlimit(RLIMIT_NPROC, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_NPROC, &limit);
}

int main(void) {
    test01();
    test02();
    test03();
    test04();
    test05();

    return 0;
}
