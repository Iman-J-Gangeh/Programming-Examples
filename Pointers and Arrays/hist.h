
/* returns the number of columns in a row */
int num_cols(char[]); 

/* 
    returns 2 if quotation marks, 0 otherwise 
    creates a string that contains the values seperated by null terminators
*/
int clean_str(char[], char[]);

/* 
    creates an array of values 
    return the size of the array
*/
void str_to_arr(char[], int[], int cols);

int sum_arr(int[], int);

void print_hist(int[], int, int);

int round_down(int);

int round_up(int);


