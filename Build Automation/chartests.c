/* Tests functions for converting characters between DOS and UNIX.
 * CSC 357, Assignment 1
 * Given tests, Spring '25 */

#include <stdio.h>
#include <assert.h>
#include "chars.h"

/* test01: Tests converting trivial newlines and carriage returns. */
void test01(void) {
    char str[3];

    /* NOTE: A carriage return followed by a newline can eventually be
     *       converted into a newline. */

    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');

    /* NOTE: A newline can immediately be converted into a carriage return
     *       followed by a newline. */

    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

/* test02: Tests converting a carriage return into a newline. */
void test02(void) {
    char str[3];

    /* NOTE: Ordinary characters are not converted. */

    assert(dtou('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');
}

/* test03: Tests converting multiple carriage returns into newlines. */
void test03(void) {
    char str[3];

    /* NOTE: Other whitespace is not converted. */

    assert(dtou('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(dtou(' ', str, '?') == 1);
    assert(str[0] == ' ');
    assert(str[1] == '\0');
    assert(dtou('b', str, '?') == 1);
    assert(str[0] == 'b');
    assert(str[1] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');

    /* NOTE: A second return is converted into a second newline. */

    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');
}

/* test04: Tests converting newlines into newlines. */
void test04(void) {
    char str[3];

    /* NOTE: Non-newlines reset an outstanding return. */

    assert(dtou('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('b', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == 'b');
    assert(str[2] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\r', str, '?') == 1);
    assert(str[0] == '\r');
    assert(str[1] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');

    /* NOTE: Standalone newlines are no different from ordinary characters. */

    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');
}

/* test05: Tests converting unprintables into newlines. */
void test05(void) {
    char str[3];

    /* NOTE: An unprintable that defaults to an ordinary character counts as an
     *       ordinary character, which resets an outstanding return; an
     *       unprintable that defaults to a newline counts as a newline. */

    assert(dtou('\0', str, '?') == 1);
    assert(str[0] == '?');
    assert(str[1] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\0', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '?');
    assert(str[2] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\0', str, '\n') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');

    /* NOTE: An unprintable that defaults to another unprintable does nothing
     *       and does not affect an outstanding return; an unprintable that
     *       defaults to a return may be converted into a newline. */

    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(dtou('\0', str, '\0') == 0);
    assert(str[0] == '\0');
    assert(dtou('\0', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '?');
    assert(str[2] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');
    assert(dtou('\0', str, '\r') == 0);
    assert(str[0] == '\0');
    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');
}

/* test06: Tests converting a newline into a carriage return. */
void test06(void) {
    char str[3];

    /* NOTE: Ordinary characters are not converted. */

    assert(utod('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

/* test07: Tests converting multiple newlines into carriage returns. */
void test07(void) {
    char str[3];

    /* NOTE: Other whitespace is not converted. */

    assert(utod('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(utod(' ', str, '?') == 1);
    assert(str[0] == ' ');
    assert(str[1] == '\0');
    assert(utod('b', str, '?') == 1);
    assert(str[0] == 'b');
    assert(str[1] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');

    /* NOTE: A second newline is converted into a second return. */

    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

/* test08: Tests converting carriage returns into carriage returns. */
void test08(void) {
    char str[3];

    /* NOTE: Non-newlines reset an outstanding return. */

    assert(utod('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(utod('b', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == 'b');
    assert(str[2] == '\0');
    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(utod('\r', str, '?') == 1);
    assert(str[0] == '\r');
    assert(str[1] == '\0');
    assert(utod('c', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == 'c');
    assert(str[2] == '\0');

    /* NOTE: Existing returns followed by newlines are not converted. */

    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

/* test09: Tests converting unprintables into carriage returns. */
void test09(void) {
    char str[3];

    /* NOTE: An unprintable that defaults to an ordinary character counts as an
     *       ordinary character, which resets an outstanding return; an
     *       unprintable that defaults to a return counts as a return. */

    assert(utod('\0', str, '?') == 1);
    assert(str[0] == '?');
    assert(str[1] == '\0');
    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(utod('\0', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '?');
    assert(str[2] == '\0');
    assert(utod('\0', str, '\r') == 0);
    assert(str[0] == '\0');
    assert(utod('\0', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '?');
    assert(str[2] == '\0');

    /* NOTE: An unprintable that defaults to another unprintable does nothing
     *       and does not affect an outstanding return; an unprintable that
     *       defaults to a newline may convert into a return. */

    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');
    assert(utod('\0', str, '\0') == 0);
    assert(str[0] == '\0');
    assert(utod('\0', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '?');
    assert(str[2] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
    assert(utod('\0', str, '\n') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

/* test10: Tests converting carriage returns and newlines concurrently. */
void test10(void) {
    char str[3];

    /* NOTE: Converting returns into newlines does not affect any outstanding
     *       returns for converting newlines into returns, and vice versa. */

    assert(dtou('a', str, '?') == 1);
    assert(str[0] == 'a');
    assert(str[1] == '\0');
    assert(dtou('\r', str, '?') == 0);
    assert(str[0] == '\0');

    assert(utod('b', str, '?') == 1);
    assert(str[0] == 'b');
    assert(str[1] == '\0');
    assert(utod('\r', str, '?') == 0);
    assert(str[0] == '\0');

    assert(dtou('\n', str, '?') == 1);
    assert(str[0] == '\n');
    assert(str[1] == '\0');

    assert(utod('c', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == 'c');
    assert(str[2] == '\0');
    assert(utod('\n', str, '?') == 2);
    assert(str[0] == '\r');
    assert(str[1] == '\n');
    assert(str[2] == '\0');
}

int main(void) {
    test01();
    test02();
    test03();
    test04();
    test05();
    test06();
    test07();
    test08();
    test09();
    test10();

    return 0;
}
