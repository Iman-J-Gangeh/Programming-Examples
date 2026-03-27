# include <stdio.h>
# include "dict.h"


void initialize_dict(Dict *dct); /* initializes dict to hold all strings of length 1*/
void pack(FILE *dest, int *codes, int codes_size); /* packs codes into bytes and writes to file */
int *unpack(FILE *src, int *out_count); /* unpacked bytes into int codes */
char *hex_to_str(int code); /* turns hex int to str */
void insert_dict(Dict *dct, int code, char *str); /* helper function to insert into dictionary */
void initialize_decode_dict(Dict *dct); /* initializes decode dictionary to hold all strings */
char *concat_prev(char *prev, char first); /* concatenates a string and a char */




