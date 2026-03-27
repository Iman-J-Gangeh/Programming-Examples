/* Defines functions for sorting arrays in parallel.
 * CSC 357, Assignment 7
 * Given code, Winter '24 */

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

# include "fsort.h"

# include <errno.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>

int write_to_buffer(int fd, void *buf, int num_bytes) {
    char *ptr;
    int bytes_left;
    int bytes;

    ptr = (char *)buf;
    bytes_left = num_bytes;

    while (bytes_left > 0) {
        bytes = write(fd, ptr, bytes_left);
        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        ptr += bytes;
        bytes_left -= bytes;
    }

    return 0;
}

int read_buffer(int fd, void *buf, int num_bytes) {
    char *ptr;
    int bytes_left;
    int bytes;

    ptr = (char *)buf;
    bytes_left = num_bytes;

    while (bytes_left > 0) {
        bytes = read(fd, ptr, bytes_left);
        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (bytes == 0) {
            return -1;
        }
        ptr += bytes;
        bytes_left -= bytes;
    }

    return 0;
}

void merge(void *base, int left_size, int right_size, int width, int (*cmp)(const void *, const void *)) {
    int arr_size;
    int bytes;
    char *arr;
    char *temp;

    arr_size = left_size + right_size;
    bytes = arr_size * width;
    arr = (char *)base;

    temp = (char *)malloc(bytes);
    if (!temp) {
        qsort(base, arr_size, width, cmp);
        return;
    }

    {
        int i = 0;
        int j = 0;
        int k = 0;

        while (i < left_size && j < right_size) {
            void *first = arr + (i * width);
            void *second = arr + ((left_size + j) * width);

            if (cmp(first, second) <= 0) {
                memcpy(temp + (k * width), first, width);
                i++;
            }
            else {
                memcpy(temp + (k * width), second, width);
                j++;
            }
            k++;
        }

        while (i < left_size) {
            memcpy(temp + (k * width), arr + (i * width), width);
            i++;
            k++;
        }

        while (j < right_size) {
            memcpy(temp + (k * width), arr + ((left_size + j) * width), width);
            j++;
            k++;
        }
    }

    memcpy(arr, temp, bytes);
    free(temp);
}

void outer(void *base, int size, int width, int (*cmp)(const void *, const void *)) {
    int left_size;
    int right_size;
    char *arr;

    if (size <= 1) {
        return;
    }

    left_size = size / 2;
    right_size = size - left_size;
    arr = (char *)base;

    outer(arr, left_size, width, cmp);
    outer(arr + (left_size * width), right_size, width, cmp);
    merge(arr, left_size, right_size, width, cmp);
}


/* fsort: Sorts an array using a parallelized merge sort.
 * TODO: Implement this function. It should sort the "n" elements of "base",
 *       each of "width" bytes, creating a child process to sort the latter
 *       half of the array if and only if "n > min", and returning 1 if the
 *       requisite child processes could not be created and 0 otherwise. */
int fsort(
 void *base, size_t n, size_t width, size_t min,
 int (*cmp)(const void *, const void *))
{
    int left_size;
    int right_size;
    char *arr;
    char *mid;
    int right_bytes_occupied;
    int fds[2];
    pid_t pid;
    int error_flag;

    if (n <= 1) {
        return 0;
    }

    if (n <= min) {
        outer(base, n, width, cmp);
        return 0;
    }

    left_size = n / 2;
    right_size = n - left_size;            /* odd give to right */
    arr = (char *)base;
    mid = arr + (left_size * width);
    right_bytes_occupied = right_size * width;

    if (pipe(fds) != 0) {
        outer(base, n, width, cmp);
        return 0;
    }

    pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return 1;
    }

    if (pid == 0) {
        int child_fail_flag;

        close(fds[0]);

        child_fail_flag = fsort(mid, right_size, width, min, cmp);

        if (write_to_buffer(fds[1], mid, right_bytes_occupied) != 0) {
            child_fail_flag = 1;
        }

        close(fds[1]);

        if (child_fail_flag) {
            _exit(1);
        }
        else {
            _exit(0);
        }
    }

    /* parent */
    close(fds[1]);

    error_flag = 0;

    if (fsort(arr, left_size, width, min, cmp) != 0) {
        error_flag = 1;
    }

    if (read_buffer(fds[0], mid, right_bytes_occupied) != 0) {
        error_flag = 1;
    }

    close(fds[0]);

    {
        int status;

        status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            error_flag = 1;
        }
        else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            error_flag = 1;
        }
    }

    merge(arr, left_size, right_size, width, cmp);

    if (error_flag) {   
        return 1;
    }
    else {
        return 0;
    }
}


