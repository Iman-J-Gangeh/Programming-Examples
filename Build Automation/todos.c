/* Converts characters from UNIX to DOS.
 * CSC 357, Assignment 1
 * Given code, Spring '25 */

#include <stdio.h>
#include "chars.h"

int main(void) {
    char next, str[3];
    while (scanf("%c", &next) != EOF) {
        /* TODO: Call "utod" here, then print the result. */
        utod(next, str, '\0');
        printf("%s", str);
    }

    return 0;
}
