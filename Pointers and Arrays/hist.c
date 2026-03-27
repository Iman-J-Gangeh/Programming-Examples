# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include "hist.h"

# define ENTRIES 8192
# define HIST_RANGE 4096
# define OFFSET 90
# define MIN_RANGE -90
# define MAX_RANGE 990 


int main(void) {
    int status = 0; 
    int cols;
    
    char in[ENTRIES];
    char cleaned[ENTRIES];
    int values[ENTRIES];

    int hist[HIST_RANGE] = {0}; 
    int size;
    int sum;
    int min = 990;
    int max = -90;
    /* gets the first line and finds the num of columns */
    fgets(in, sizeof(in), stdin);
    cols = num_cols(in);
    /* gets the rest of the rows and processes them into num array */
    while(fgets(in, sizeof(in), stdin)) {
        status = clean_str(in, cleaned);  
        if(status == 2) {
            printf("./hist: Unsupported quotes\n");
            return status;
        }
        size = num_cols(in);
        if(size != cols) {
            status = 1;
            printf("./hist: Mismatched cells\n");
            return status;
        } 
        else {
            /* process string into an array */
            str_to_arr(cleaned, values, cols);
            
            sum = sum_arr(values, size);

            if(sum <= min) {
                min = sum;
            }
            if(sum >= max) {
                max = sum;
            }
            ++hist[sum+90];
        }
    }
    print_hist(hist, min, max);

    return status; 
}

/* 
    count the number of distinct comma seperated strings  
    the number of cols is equal to the number of commas+1
*/
int num_cols(char str[]) {
    int count = 0; 
    int i; 
    for(i = 0; str[i] != '\0'; ++i) {
        if(str[i] == ',') {
            count++; 
        }
    }
    return count+1; 
}

/* 
    calls functions to clean string 
*/
int clean_str(char str[], char cleaned[]) {
    int i = 0; 
    int count = 0; 
    for(i = 0; str[i] != '\0'; ++i) {
        if(!isspace(str[i])) {
            if(str[i] == '"' || str[i] == '\'') {
                return 2;
            }
            else if(str[i] == ',') {
                cleaned[count] = '\0';
            }
            else {
                cleaned[count] = str[i];
            }
            ++count;
        }
    }
    cleaned[count] = '\0';
    cleaned[count+1] = '\0';
    
    return 0;
}



void str_to_arr(char str[], int values[], int cols) {
    char *end; 
    int i = 0;
    int placeholder; 
    
    while(i < cols) {
        while(str[0] == '\0') {
            ++str;
        }
        
        placeholder = strtol(str, &end, 10);
        
        if(str == end) {
            while(end[0] != '\0') {
                ++end;
            }
            values[i] = 0;
        }
        else {
            values[i] = placeholder;
        }
        str = end;
        if(str[0] != '\0') {
            ++str;
        }
        ++i;
    }
    
}


int sum_arr(int values[], int size) {
    int i = 0; 
    int sum = 0; 
    for(i =0 ; i < size; ++i) {
        sum += values[i];
    }
    return sum;
}


void print_hist(int hist[], int min, int max) {
    int num_bins; 
    int max_bin = 0;
    int i;
    int j; 
    int b; 
    int r; 
    int l; 
    int k; 
    int left_count = 0;
    int right_count = 0;
    int height;
    int width;
    int count;
    char line[32768];
    int col;
    int start;
    char num_labels[256];
    int label_length;
    int temp; 
    int init_len; 


    min = round_down(min);
    max = round_up(max);

    num_bins = (max - min) / 2 + 1;
    
    /* Calculate the height based off hist */
    for (i = 0; i < num_bins; ++i) {
        r = min + 2 * i;         
        l = r - 1;                 
        
        left_count = 0; 
        right_count = 0;

        if (l >= MIN_RANGE && l <= MAX_RANGE) {
            left_count = hist[l + OFFSET];
        }
        if (r >= MIN_RANGE && r <= MAX_RANGE) {
            right_count = hist[r + OFFSET];
        }

        count = left_count + right_count;
        if (count > max_bin) {
            max_bin = count;
        }
    }

    height = ((max_bin + 4) / 5) * 5;  

    printf("    | ");
    for (b = 0; b < num_bins; b++) {
        printf(" ");
    }
    printf(" |\n");
    
    /* Printing out values (start from top) */
    for (j = height; j >= 1; --j) {
        if (j % 5 == 0) {
            printf("%3d T", j);
        }
        else {
            printf("    |");
        }

        printf(" ");

        /* calculating bins */
        for (b = 0; b < num_bins; ++b) {
            r = min + 2 * b;
            l = r - 1;

            left_count = 0; 
            right_count = 0; 

            if (l >= MIN_RANGE && l <= MAX_RANGE) {
                left_count = hist[l + OFFSET];
            }
            if (r >= MIN_RANGE && r <= MAX_RANGE) {
                right_count = hist[r + OFFSET];
            } 

            count = left_count + right_count;

            if (count >= j) {
                printf("#");
            }
            else {
                printf(" ");
            }        
        }

        if (j % 5 == 0) {
            printf(" T %d\n", j);
        }
        else {
            printf(" |\n");
        }          
    }

    printf("    +-");
    for (b = 0; b < num_bins; ++b) {
        k = min + 2 * b;
        if (k % 10 == 0)  {
            printf("|");
        }
        else {
            printf("-");
        }            
    }
    printf("-+\n");

    if (num_bins > (int)sizeof(line) - 1) {
        width = (int)sizeof(line) - 1;
    } 
    else {
        width = num_bins;
    }

    width += 2;
    for (i = 0; i < width; i++) {
        line[i] = ' ';
    }
    line[width] = '\0'; 

    for (b = 0; b < num_bins && b < width; ++b) {
        k = min + 2 * b;

        if (k % 10 == 0) {
            sprintf(num_labels, "%d", k); 
            label_length = 0;
            temp = k;

            if (temp == 0) {
                label_length = 1;
            } 
            else {
                if (temp < 0) {
                    label_length++;     
                    temp = -temp;
                }
                while (temp > 0) {
                    label_length++;
                    temp /= 10;
                }
            }
            if(b == 0) {
                init_len = label_length; 
            }
            
        
            start = b - (label_length - 3);
            
            for (col = 0; num_labels[col] != '\0'; ++col) {
                int pos = start + col;
                if (pos >= 0 && pos < width) {
                    line[pos] = num_labels[col];
                }
            }
        }
    }
    
    
    if(init_len == 3) {
        printf("    %s\n", line);  
    }
    else if(init_len == 2) {
        printf("    %s\n", line);  
    }
    else {
        printf("    %s\n", line);  
    }
    

    width = 1 * num_bins;               
    if (width > (int)sizeof(line) - 1) {
        width = (int)sizeof(line) - 1;
    }

}

int round_down(int num) {
    if(num >= 0) {
        return num / 10 * 10;
    }
    else {
        if(num % 10 == 0) {
            return num;
        }
        else {
            return (num / 10-1) * 10; 
        }
    }
}

int round_up(int num) {
    if(num >= 0 ) {
        if(num % 10 == 0) {
            return num;
        }
        else {
            return (num / 10+1) * 10; 
        }
    }
    else {
        return num / 10 * 10; 
    }
    return num;
}
