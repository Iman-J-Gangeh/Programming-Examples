#include "batch.h"

int error = 0;
int signal_recieved = 0;

int main(int argc, char *argv[]) {
    Options option;
    int start;
    int num_commands;
    Command *commands;

    option.n_flag = 0;
    option.num_commands = 0;
    option.e_flag = 0;
    option.v_flag = 0;

    if (parse_commands(argc, argv, &option, &start) != 0) {
        error_message(argv[0]);
        return EXIT_FAILURE;
    }

    if (start >= argc || strcmp(argv[start], "--") != 0) {
        error_message(argv[0]);
        return EXIT_FAILURE;
    }

    commands = get_command(argc, argv, start, &num_commands);
    if (!commands) {
        error_message(argv[0]);
        return EXIT_FAILURE;
    }

    if (!option.n_flag) {
        option.num_commands = num_commands;
    }

    create_handlers();

    {
        int next = 0;
        int active = 0;
        int end = 0;
        int pgid = -1;

        while (1) {
            if (signal_recieved) {
                error = 1;
                end = 1;
            }

            while (!end && active < option.num_commands && next < num_commands) {
                int pid;

                if (option.v_flag) {
                    print_command(stderr, "+ ", commands[next].argv);
                }

                pid = fork();

                if (pid == 0) {
                    execvp(commands[next].argv[0], commands[next].argv);
                    perror(commands[next].argv[0]);
                    exit(EXIT_FAILURE);
                }

                if (pgid < 0) pgid = pid;
                (void)setpgid(pid, pgid);

                commands[next].pid = pid;
                active++;
                next++;
            }

            if (end) {
                if (pgid > 0) {
                    killpg(pgid, SIGKILL);
                }
            }

            if (active == 0) {
                break;
            }

            {
                int status;
                int done = waitpid(-1, &status, 0);

                if (done < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    error = 1;
                    break;
                }

                {
                    int id = find_process(commands, num_commands, done);
                    if (id >= 0) {
                        if (option.v_flag) {
                            print_command(stderr, "- ", commands[id].argv);
                        }
                        commands[id].pid = -1;
                    }
                }

                active--;

                if (!succeeded(status)) {
                    error = 1;
                    if (option.e_flag) {
                        end = 1;
                    }
                }
            }
        }
    }

    free_commands(commands, num_commands);

    if (error) {
        return EXIT_FAILURE;
    } 
    else {
        return EXIT_SUCCESS;
    }
}

void free_commands(Command *commands, int num_commands) {
    int i;

    for (i = 0; i < num_commands; i++) {
        free(commands[i].argv);
    }
    free(commands);
}

Command *get_command(int argc, char *argv[], int start, int *out) {
    int i = start;
    int groups = 0;

    while (i < argc) {
        if (strcmp(argv[i], "--") != 0) {
            return NULL;
        }
        i++;
        if (i >= argc) {
            return NULL;
        }
        groups++;
        while (i < argc && strcmp(argv[i], "--") != 0) {
            i++;
        }
    }

    if (groups < 1) {
        return NULL;
    } 

    /* extract command */
    
    Command *commands = malloc((int)groups * sizeof(Command));
    int index; 
    int j, length, k;
    i = start;

    for (index = 0; index < groups; index++) {
        i++; 
        j = i;
        while (j < argc && strcmp(argv[j], "--") != 0) {
            j++;
        }

        length = j - i;

        commands[index].argv = malloc((length + 1) * sizeof(char *));

        for (k = 0; k < length; k++) {
            commands[index].argv[k] = argv[i + k];
        }

        commands[index].argv[length] = NULL;  
        commands[index].pid = -1;          

        i = j;
    }   
    

    *out = groups;
    return commands;
}

void error_message(char *program) {
    fprintf(stderr, "usage: %s [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]\n", program);
}

int get_int(char *str, int *out) {
    char *end = NULL;
    int value;

    errno = 0;
    value = strtol(str, &end, 10);

    if (errno != 0) {
        return -1;
    }
    if (end == str || *end != '\0') {
        return -1;
    }
    if (value < 1) {
        return -1;
    }

    *out = value;
    return 0;
}

void on_signal(int sig) {
    (void)sig;
    signal_recieved = 1;
}

void create_handlers(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));

    action.sa_handler = on_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT,  &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

void print_command(FILE *out, char *prefix, char **argv) {
    int i;

    fwrite(prefix, 1, strlen(prefix), out);

    for (i = 0; argv[i] != NULL; i++) {
        if (i)
            fwrite(" ", 1, 1, out);

        fwrite(argv[i], 1, strlen(argv[i]), out);
    }

    fwrite("\n", 1, 1, out);
}

int find_process(Command *commands, int limit, int pid) {
    int i;
    for (i = 0; i < limit; i++) {
        if (commands[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

int succeeded(int status) {
    return (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS);
}

int parse_commands(int argc, char *argv[], Options *option, int *start) {
    int i = 1;

    while (i < argc) {
        if (strcmp(argv[i], "--") == 0) {
            *start = i;  
            return 0;
        }

        if (strcmp(argv[i], "-e") == 0) {
            option->e_flag = 1;
            i++;
            continue;
        }

        if (strcmp(argv[i], "-v") == 0) {
            option->v_flag = 1;
            i++;
            continue;
        }

        if (strcmp(argv[i], "-n") == 0) {
            int n;
            if (i + 1 >= argc || get_int(argv[i + 1], &n) != 0) {
                return -1;
            }
            option->n_flag = 1;
            option->num_commands = n;
            i += 2;
            continue;
        }

        return -1;
    }

    return -1;
}