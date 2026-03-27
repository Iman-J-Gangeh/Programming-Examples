#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

# include "tree.h"

static int error = 0;


int main(int argc, char *argv[]) {
    Options option;
    char *root = ".";
    int c;

    option.show_all = 0;
    option.long_mode = 0;
    option.depth_mode = 0;
    option.max_depth = 0;

    opterr = 0;
    while ((c = getopt(argc, argv, "ad:l")) != -1) {
        if (c == 'a') {
            option.show_all = 1;
        } 
        else if (c == 'l') {
            option.long_mode = 1;
        } 
        else if (c == 'd') {
            int n;
            if (!optarg || parse_int(optarg, &n) != 0) {
                fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
                return EXIT_FAILURE;
            }
            option.depth_mode = 1;
            option.max_depth = n;
        } 
        else {
            fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");

            return EXIT_FAILURE;
        }
    }

    if (optind == argc) root = ".";
    else if (optind + 1 == argc) {
        root = argv[optind];
    }
    else {
        fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
        return EXIT_FAILURE;
    }

    visit(&option, root, 0);

    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}


int parse_int(char *str, int *out) {
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

    *out = (int)value;
    return 0;
}

int skip(Options *opt, const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return 1;
    }

    if (!opt->show_all && name[0] == '.')  {
        return 1;   
    }
    return 0;
}

void lead_indent(int depth) {
    int i;
    for (i = 0; i < depth; i++) {
        fwrite("    ", 1, 4, stdout);   
    }
}

void post_indent(int depth) {
    int i;
    int n = depth * 4 + 1;

    for (i = 0; i < n; i++) {
        fwrite(" ", 1, 1, stdout); 
    }
}

char file_type(mode_t file_info) {
    if (S_ISLNK(file_info)) {
        return 'l';
    }
    if (S_ISDIR(file_info))  {
        return 'd';
    }
    else {
        return '-';
    }
}

char permissions(mode_t file_info, mode_t execute, mode_t extra, char on_flag, char off_flag) {
    if (file_info & extra) {
        if (file_info & execute) {
            return on_flag;
        } 
        else {
            return off_flag;
        }
    } 
    else {
        if (file_info & execute) {
            return 'x';
        } 
        else {
            return '-';
        }
    }
}

void perms_string(mode_t execute, char out[10]) {

    if (execute & S_IRUSR) {
        out[0] = 'r';
    }
    else {
        out[0] = '-';
    }

    if (execute & S_IWUSR) {
        out[1] = 'w';
    }
    else {
        out[1] = '-';
    }

    out[2] = permissions(execute, S_IXUSR, S_ISUID, 's', 'S');

    if (execute & S_IRGRP) {
        out[3] = 'r';
    }
    else {
        out[3] = '-';
    }

    if (execute & S_IWGRP) {
        out[4] = 'w';
    }
    else {
        out[4] = '-';
    }

    out[5] = permissions(execute, S_IXGRP, S_ISGID, 's', 'S');

    if (execute & S_IROTH) {
        out[6] = 'r';
    }
    else {
        out[6] = '-';
    }

    if (execute & S_IWOTH) {
        out[7] = 'w';
    }
    else {
        out[7] = '-';
    }

    if (execute & S_ISVTX) {
        if (execute & S_IXOTH) {
            out[8] = 't';
        }
        else {
            out[8] = 'T';
        }
    }
    else {
        if (execute & S_IXOTH) {
            out[8] = 'x';
        }
        else {
            out[8] = '-';
        }
    }

    out[9] = '\0';
}

char *pathtostr(char *fname, struct stat *list) {
    size_t size;
    ssize_t num_read;
    char *buffer;
    if (list->st_size > 0) {
        size = (size_t)list->st_size;
    } 
    else {
        size = 1;
    }    
    buffer = (char *)malloc(size + 1);
    

    num_read = readlink(fname, buffer, size);
    if (num_read < 0) {
        free(buffer);
        return NULL;
    }
    buffer[num_read] = '\0';
    return buffer;
}

void print_line(Options *option, int depth, char *name, struct stat *list, int lstat_flag, char *suffix, char *error_message) {
    
    if (!option->long_mode) {
        lead_indent(depth);

        fwrite(name, 1, strlen(name), stdout);

        if (suffix) {
            fwrite(suffix, 1, strlen(suffix), stdout);
        }

        if (error_message) {
            fwrite(": ", 1, 2, stdout);
            fwrite(error_message, 1, strlen(error_message), stdout);
        }

        fwrite("\n", 1, 1, stdout);
    }
    else {
        if (lstat_flag) {
            char type;
            char p[10];
            perms_string(list->st_mode, p);

            type = file_type(list->st_mode);
            fwrite(&type, 1, 1, stdout);
            fwrite(p, 1, strlen(p), stdout);
        } 
        else {
            fwrite("??????????", 1, 10, stdout);
        }

        post_indent(depth);

        fwrite(name, 1, strlen(name), stdout);

        if (suffix)
            fwrite(suffix, 1, strlen(suffix), stdout);

        if (error_message) {
            fwrite(": ", 1, 2, stdout);
            fwrite(error_message, 1, strlen(error_message), stdout);
        }

        fwrite("\n", 1, 1, stdout);
    }    
}

int compare_fct(const void *a, const void *b) {

    const char *x = *(const char * const *)a;
    const char *y = *(const char * const *)b;
    return strcmp(x, y);
}

char **read_directories(Options *options, int *out_len) {
    DIR *dir;
    struct dirent *entry;
    int cap = 5;
    int len = 0;
    char **names = (char **)malloc(cap * sizeof(char *));

    dir = opendir(".");
    
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (skip(options, entry->d_name)) {
            continue;
        }

        if (len >= cap) {
            cap = 2 * cap + 1;
            names = (char **)realloc(names, cap * sizeof(char *));
        }

        names[len++] = strdup(entry->d_name);
        if (!names[len - 1]) {
            closedir(dir);
            *out_len = len;
            return names;
        }
    }

    if (errno != 0) {
        error = 1;
    }

    closedir(dir);

    qsort(names, len, sizeof(char *), compare_fct);
    *out_len = len;

    return names;
}


void visit(Options *option, char *name, int depth) {
    struct stat st;
    int lstat_flag;
    char *suffix = NULL;

    if (option->depth_mode && depth > option->max_depth) {
        return;
    }

    lstat_flag = (lstat(name, &st) == 0);
    if (!lstat_flag) {
        error = 1;
        print_line(option, depth, name, NULL, 0, NULL, strerror(errno));
        return;
    }

    if (option->long_mode && S_ISLNK(st.st_mode)) {
        char *link = pathtostr(name, &st);
        if (!link) {
            error = 1;
            print_line(option, depth, name, &st, 1, NULL, strerror(errno));
            return;
        }
        {
            size_t need = strlen(" -> ") + strlen(link) + 1;
            suffix = (char *)malloc(need);
            if (suffix)  {
                snprintf(suffix, need, " -> %s", link);
            }
        }
        free(link);
    }

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(name);
        if (!d) {
            error = 1;
            print_line(option, depth, name, &st, 1, suffix, strerror(errno));
            free(suffix);
            return;
        }
        closedir(d);
    }

    print_line(option, depth, name, &st, 1, suffix, NULL);
    free(suffix);

    if (!S_ISDIR(st.st_mode)) {
        return;
    }
    if (option->depth_mode && depth >= option->max_depth) return;

    {
        int saved_fd = open(".", O_RDONLY);
        if (saved_fd < 0) {
            error = 1;
            return;
        }

        if (chdir(name) != 0) {
            error = 1;
            close(saved_fd);
            return;
        }

        {
            int n;
            int i;
            char **kids = read_directories(option, &n);
            if (!kids) {
                error = 1;
            } else {
                for (i = 0; i < n; i++) {
                    if (kids[i]) {
                        visit(option, kids[i], depth + 1);
                        free(kids[i]);
                    }
                }
                free(kids);
            }
        }

        fchdir(saved_fd);
        close(saved_fd);
    }
}




/* practice of writing to a file */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


# define SIZE 4096
int main(int argc, char *argv[]) {
    char buf[SIZE];
    int n, src, dest; 

    src = open(argv[1], O_RDONLY);
    dest = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 
                S_IRUSR | S_IWUSR | S_IXUSR | S_IOWNR);




}
