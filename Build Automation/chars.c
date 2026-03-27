/* Defines functions for converting characters between DOS and UNIX.
 * CSC 357, Assignment 1
 * Reference sol'n, Spring '25. */

#include "chars.h"
#include <stdio.h>

/* istext: Checks whether or not a character is plain text.
 * NOTE: Do not alter this function. It returns 1 if "c" has an ASCII code in
 *       the ranges [0x08, 0x0D] or [0x20, 0x7E], and 0 otherwise. */
int istext(char c) {
    return (c >= 0x08 && c <= 0x0D) || (c >= 0x20 && c <= 0x7E);
}

/* dtou: Converts a character from DOS to UNIX.
 * TODO: Implement this function. It should return the number of characters
 *       placed into "str": generally nothing if "next" is a carriage return,
 *       a newline if "next" is a newline following a carriage return, and
 *       "next" otherwise. Use "dflt" instead if "next" is not plain text, and
 *       do nothing if neither is plain text. See also the given unit tests. */
int dtou(char next, char str[], char dflt) {
    static int cr = 0;
    int count = 0;
 
    if(next=='\n') { 
        cr = 0;
        str[0] = next;
        count++;
    }
    else if(next=='\r') { 
        if(cr) {
            str[0] = '\r';
            count++;
        }
        cr = 1;
    }
    else if(!istext(next)) { 
        if(dflt=='\n') {
            str[0] = '\n';
            cr = 0; 
            count++;
        }
        else if(dflt=='\r'){
            if(cr) {
                str[0] = '\r';
            } 
            cr = 1;
        }
        else if (istext(dflt)) { 
            if(cr) {
                str[0] = '\r';
                str[1] = dflt;
                cr = 0;
                count = 2;
            }
            else {
                str[0] = dflt;
                count++;
            }
        }
    }
    else {  
        if(cr) {
            str[0] = '\r';
            str[1] = next;
            cr = 0;
            count++;
        }
        else {
            str[0] = next; 
        }
        count++;
    } 
    str[count] = '\0';

    return count;
}

/* utod: Converts a character from UNIX to DOS.
 * TODO: Implement this function. It should return the number of characters
 *       placed into "str": generally a carriage return followed by a newline
 *       if "next" is a newline, and "next" otherwise. Use "dflt" instead if
 *       "next" is not plain text, and do nothing if neither is plain text. See
 *       also the given unit tests. */
int utod(char next, char str[], char dflt) {
    static int cr = 0; 
    int count = 0;
    
    if(next=='\n') {
        str[0] = '\r';
        str[1] = '\n';
        count = 2;
        cr = 0;
    }
    else if(next=='\r') {
        if(cr) {
            str[0] = '\r';
            count++;
        }
        cr = 1;
    }
    else if(!istext(next)) {
        if(dflt=='\n') {
            str[0] = '\r';
            str[1] = '\n';
            count = 2; 
            cr = 0; 
        }
        else if(dflt=='\r') {
            if(cr) {
                str[0] = '\r';
                count++;
            }
            cr=1;
        }
        else if(istext(dflt)) {
            if(cr) {
                str[0] = '\r';
                str[1] = dflt;
                count = 2;
                cr = 0;

            }
            else {
                str[0] = dflt ;
                count++; 
            }
        }
    }
    else {
        if(cr) {
            str[0] = '\r';
            str[1] = next;
            cr = 0;
            count = 2;
        }
        else {
            str[0] = next;
            count++; 
        }
    }
    str[count] = '\0';

    return count; 
} 