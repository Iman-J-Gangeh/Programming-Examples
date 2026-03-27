# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include "dict.h"
# include "freq.h"

# define BUFFER 1024
# define WHITESPACE "\t\n\v\f\r "

int main(int argc, char *argv[]) {
    FILE *f;
    char buffer[BUFFER];
    char *text = NULL; 
    char *text_end = NULL; 
    int size = 0; 
    Dict *dict; 
    char *tmp;

    /* initialize diction */
    dict = dctcreate();

    /* Check for correct number of arguments */
    if(argc < 2) {
        printf("./freq: Too few arguments\n");
        return 1;
    }
    else if(argc > 2) {
        printf("./freq: Too many arguments\n");
        return 1;
    }
    
    f = fopen(argv[1], "r");

    if(f == NULL) {
        printf("./freq: No such file or directory\n");
        return 1; 
    }

    /* reads text from file into buffer adds it to a dynamic string */
    while(fgets(buffer, BUFFER, f)) {
        int len = strlen(buffer); 
        
        tmp = (char *) realloc(text, size + len + 1);
        if(!tmp) {
            free(text);
            printf("Reallocate Failed");
            return 1; 
        }
        text = tmp;

        text_end = text + size; 
        
        strcpy(text_end, buffer);
        
        size += strlen(buffer);
    }

    /* tokenize strings */
    {
        int i; 
        char *str ;
        char *token = strtok(text, WHITESPACE);
        while(token) {

            /* clean the token */
            for(i = 0; token[i]; ++i) {
                token[i] = tolower(token[i]);  /* removes capitals */
            }

            str = token;
            for(i = 0; token[i]; ++i) {
                if(isalpha(token[i])) {
                    *str++ = token[i]; /* removes non alphabet characters */
                }
            }
            *str = '\0';

            /* end of cleaning token */

            /* store token in the frequency dict */
            if(token[0] != '\0') {
                int *count = dctget(dict, token);
                if (count == NULL) {
                    count = malloc(sizeof *count);
                    *count = 1;
                    dctinsert(dict, token, count);
                } 
                else {
                    (*count)++;
                }
            }
            token = strtok(NULL, WHITESPACE);

        }
    }

    /* find the 10 most frequent words and their frequencies*/
    {
        int i; 
        void *val; 
        Pair *pairs = (Pair *)malloc(dict->size * sizeof(Pair));
        char **keys = dctkeys(dict);
        for(i = 0; i < dict->size; ++i) {
            val = dctget(dict, keys[i]);

            pairs[i].key = keys[i];
            pairs[i].val = *(int *)val;
        }

        qsort(pairs, dict->size, sizeof(Pair), &compare_vals);

        /* print out */ 
        {
            int size; 
            int i;
            if(dict->size < 10) {
                size = dict->size;
            }       
            else {
                size = 10;
            }

            for(i=0; i<size; ++i) {
                printf("%s (%d)\n", pairs[i].key, pairs[i].val);
            }
        }
         

        free(pairs);
        free(keys);
    }


    {
        int i; 
        char **keys2 = dctkeys(dict);
        for (i = 0; i < dict->size; i++) {
            free(dctget(dict, keys2[i]));
        }
        free(keys2);
    }  

    dctdestroy(dict);
    free(tmp);
    fclose(f);

    return 0; 
}

int compare_vals(const void *a, const void *b) {
    const Pair *pa = (const Pair *)a;
    const Pair *pb = (const Pair *)b;


    if(pa->val < pb->val) return 1; 
    if(pa->val > pb->val) return -1;

    return strcmp(pa->key, pb->key);
}