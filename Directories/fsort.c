    /* Defines functions for sorting arrays in parallel.
    * CSC 357, Assignment 7
    * Given code, Winter '24 */

#include "fsort.h"

/* fsort: Sorts an array using a parallelized merge sort.
 * TODO: Implement this function. It should sort the "n" elements of "base",
 *       each of "width" bytes, creating a child process to sort the latter
 *       half of the array if and only if "n > min", and returning 1 if the
 *       requisite child processes could not be created and 0 otherwise. */
int fsort(
 void *base, size_t n, size_t width, size_t min,
 int (*cmp)(const void *, const void *))
{
    return 0;
}
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

# include "fsort.h"

# include <errno.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
