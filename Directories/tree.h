#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct {
    int show_all;     
    int long_mode;    
    int depth_mode;   
    int max_depth;    
} Options;


int parse_int(char *str, int *out); /* takes a string and places an integer into out */
int skip(Options *opt, const char *name); /* 1 if file should be skipped, 0 if not */
void lead_indent(int depth); /* prints spaces according to depth */
void post_indent(int depth); /* prints spaces according to depth after tags */
char file_type(mode_t file_info); /* returns the type of the file */
char exec_char(mode_t file_info, mode_t execute, mode_t extra, char on_flag, char off_flag); /* examine file permissions and return flag*/
void perms_string(mode_t execute, char out[10]); /* creates a permissions string */
char *pathtostr(char *fname, struct stat *list); /* converts a link to a str */
void print_line(Options *option, int depth, char *name, struct stat *list, int lstat_flag, char *suffix, char *error); /* prints each individual line*/
int compare_fct(const void *a, const void *b); /* used for qsort */
char **read_directories(Options *options, int *out_len); /* reads the directories names */
void visit(Options *option, char *name, int depth); /* visits all files recursively */


