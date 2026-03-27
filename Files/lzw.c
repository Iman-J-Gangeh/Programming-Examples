#define _POSIX_C_SOURCE 200809L
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "lzw.h"
# define MAX_CODE 0xFFF

int main(int argc, char *argv[]) {
    FILE *src, *dest;
    Dict *dct = dctcreate();
    unsigned char buf[1];
    char *prev;
    char *curr; 
    void *val;
    int n; 
    int len; 
    int codes_cap = 5; 
    int *codes = (int *) malloc(codes_cap * sizeof(int));
    int codes_size = 0;
    
    unsigned int next_code = 0x080;

 

    if(argc != 4) {
        fprintf(stderr, "usage: ./lzw (-c | -x) SOURCE DESTINATION\n");
        free(codes);
        dctdestroy(dct);
        return EXIT_FAILURE;
    }

    /* opening the source file */ 
    if((src = fopen(argv[2], "r")) == NULL) {
        perror("./lzw");
        free(codes);
        dctdestroy(dct);
        return EXIT_FAILURE;
    }

    /* opening the destination file */
    if((dest = fopen(argv[3], "w")) == NULL) {
        perror("./lzw");
        fclose(src);
        free(codes);
        dctdestroy(dct);
        return EXIT_FAILURE; 
    }
    
    if(strcmp(argv[1], "-c") == 0) {
        /* encode first file */
        
        initialize_dict(dct);
        
        /* read first letter */
        if((n = fread(buf, sizeof(char), 1, src)) > 0) {
            prev = malloc(2* sizeof(char));

            prev[0] = (char)*buf;
            prev[1] = '\0';
            
            while((n = fread(buf, sizeof(char), 1, src)) > 0) {
            
                len = strlen(prev);
                curr = malloc(len + 2);
                
                memcpy(curr, prev, len);
                curr[len] = (char)*buf;
                curr[len + 1] = '\0';

                val = dctget(dct, curr);

                if(val != NULL) {
                    free(prev); 
                    prev = curr;
                }
                else {
              
                    int *curr_code = (int *)dctget(dct, prev);
                    if(codes_size >= codes_cap) {
                        codes_cap = 2*codes_cap + 1; 
                        codes = realloc(codes, codes_cap * sizeof(int));
                    }
                    codes[codes_size++] = *curr_code; 
                    


                    /* add to dictionary */
                    {
                        int *val;
                        char *key = strdup(curr); 
                        val = malloc(sizeof(int));
                        *val = next_code;
                        
                        dctinsert(dct, key, val);
                    }

                    ++next_code;

                    /* reset to a single character */
                    free(prev);
                    prev = malloc(2*sizeof(char));

                    prev[0] = (char)*buf;
                    prev[1] = '\0';

                    free(curr);
                }
            }
            /* account for end of file */
            if (prev != NULL) {
                int *prev_code = (int *) dctget(dct, prev);

                if (codes_size >= codes_cap) {
                    codes_cap = 2 * codes_cap + 1;
                    codes = realloc(codes, codes_cap * sizeof(int));
                }

                codes[codes_size++] = *prev_code;

                free(prev);
            }
        }


        if (codes_size > 0) {
            pack(dest, codes, codes_size);
        }
        free(codes); 
        codes = NULL;
        dctdestroy(dct);         
        fclose(src);
        fclose(dest);
        /* debug
        {
            int i;
            for(i = 0; i < codes_size; ++i) {
                printf("%x", codes[i]);
            }
            printf("\n");

        }
        */
        
    }
    else if(strcmp(argv[1], "-x") == 0) {
        
        /* decode the first file */
        int i;
        int next;
        char *str;
        free(codes);
        codes = unpack(src, &codes_size);
        
        if (codes_size == 0) {
            free(codes);
            dctdestroy(dct);
            fclose(src);
            fclose(dest);
            return 0;
        }

        initialize_decode_dict(dct);

        next = 0x080;

        /* decode codes and write to  */
        {
            int code = codes[0] & 0x0FFF;
            
            char *key = hex_to_str(code);
            prev = (char *)dctget(dct, key);
            free(key);
                        
            prev = strdup(prev);
            
            fwrite(prev, 1, strlen(prev), dest);

            for (i = 1; i < codes_size; i++) {
                char *val;
                int code = codes[i] & 0x0FFF;
                char *key = hex_to_str(code);
                val = (char *)dctget(dct, key);
                free(key);

                if (val) {
                    val = strdup(val);
                } 
                else {
                    val = concat_prev(prev, prev[0]);
                } 

                fwrite(val, 1, strlen(val), dest);

                str = concat_prev(prev, val[0]);
                
                insert_dict(dct, next, str);
                free(str);
                next++;
                

                free(prev);
                prev = val;
            }

            free(prev);
        }

        dctdestroy(dct);
        free(codes);
        fclose(src); 
        fclose(dest);
    }
    else {
        fprintf(stderr, "usage: ./lzw (-c | -x) SOURCE DESTINATION\n");
        free(codes);
        dctdestroy(dct);
        return EXIT_FAILURE;
    }

    return 0; 
}

void initialize_dict(Dict *dct) {
    int code; 
    int *val;
    char *key;
    char temp[2];
    for(code = 0x000; code <= 0x07F; ++code) {
        temp[0] = (char)code;
        temp[1] = '\0';

        val = malloc(sizeof(int));
        *val = code; 

        key = strdup(temp);
        
        dctinsert(dct, key, val);
    }
}


void pack(FILE *dest, int *codes, int codes_size) {
    int i;
    int first;
    int second;
    unsigned char write[3];
    for(i = 0; i+1 < codes_size;) {
        /* Create bit codes two elements at a time */
        first = codes[i++] & 0x0FFF;
        second = codes[i++] & 0x0FFF;
        /* store 24 combined bits into the 3 bytes*/
        write[0] = (unsigned char)((first >> 4) & 0xFF);
        write[1] = (unsigned char)(((first & 0x0F) << 4) | ((second >> 8) & 0x0F));
        write[2] = (unsigned char)(second & 0xFF);
        fwrite(write, 1, 3, dest);
    }
    if (i < codes_size) {
        /* handle odd number of codes */
        first = codes[i] & 0x0FFF;
        write[0] = (unsigned char)((first >> 4) & 0xFF);
        write[1] = (unsigned char)((first & 0x0F) << 4); 
        fwrite(write, 1, 2, dest);
    }
}

int *unpack(FILE *src, int *codes_size) {
    int codes_cap = 5; 
    int size = 0;
    int *codes = malloc(codes_cap * sizeof(int));
    unsigned char input[3];
    int read;
    int first;
    int second;

    while ((read = fread(input, 1, 3, src)) == 3) {
        first = ((int)input[0] << 4) | ((int)input[1] >> 4);
        second = (((int)input[1] & 0x0F) << 8) | (int)input[2];

        if (size + 2 > codes_cap) {
            codes_cap = codes_cap * 2 + 1;
            codes = realloc(codes, codes_cap * sizeof(int));
        }
        codes[size++] = first;
        codes[size++] = second;
    }

    /* handle odd number of codes */
    if (read == 2) {
        first = ((int)input[0] << 4) | ((int)input[1] >> 4);
        if (size + 1 > codes_cap) {
            codes_cap = codes_cap * 2 + 1;
            codes = realloc(codes, codes_cap * sizeof(int));
        }
        codes[size++] = first;
    }

    *codes_size = size;

    return codes;
}

char *hex_to_str(int code) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%03x", code & 0x0FFF);
    return strdup(buffer);
}

void insert_dict(Dict *dct, int code, char *str) {
    char *key = hex_to_str(code);
    char *val = strdup(str);
    dctinsert(dct, key, val);
}

void initialize_decode_dict(Dict *dct) {
    char str[2];
    int code;
    for (code = 0x000; code <= 0x07F; code++) {
        str[0] = (char)code;
        str[1] = '\0';
        insert_dict(dct, code, str);
    }
}

char *concat_prev(char *prev, char first) {
    int len = strlen(prev);
    char *out = malloc(len + 2);
    memcpy(out, prev, len);
    out[len] = first;
    out[len + 1] = '\0';
    return out;
}

