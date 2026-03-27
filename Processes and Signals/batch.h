#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

# include <errno.h>
# include <signal.h>
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <unistd.h>

typedef struct {
    int n_flag;
    int num_commands;
    int e_flag;
    int v_flag;
} Options;

typedef struct {
    char **argv; 
    int pid;
} Command;


void free_commands(Command *commands, int m); /* memory deallocation for commands */
void error_message(char *program); 
int get_int(char *str, int *out); 
void on_signal(int sig); /* sets global signal */
void create_handlers(void); /* controls signal handling */
int find_process(Command *commands, int limit, int pid); /* search for a specific process*/
void print_command(FILE *out, char *prefix, char **argv); 
int succeeded(int status); /* true if successful process exiting */
Command *get_command(int argc, char *argv[], int start, int *out); /* parses a command */
int parse_commands(int argc, char *argv[], Options *option, int *start); /* parses commands from argv */




