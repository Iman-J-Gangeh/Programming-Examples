#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
# include <errno.h>
# include <fcntl.h>
# include <limits.h>
# include <netdb.h>
# include <poll.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <time.h>
# include <unistd.h>


#define LISTEN_LIMIT 128
#define READ_SIZE 409

typedef struct buffer {
    char *data;
    int len;
    int cap;
} Buffer;

typedef struct peer {
    int fd;
    Buffer out;
} Peer;

typedef struct peers {
    struct peer *data;
    int len;
    int cap;
} Peers;

typedef struct arguments {
    char *prog;
    char *username;
    int timeout;
    char *local_port;
    char *remote_host;
    char *remote_port;
} Arguments;



int error_out(char *prog); /* error message */
int buffer_cap(Buffer *buf, int need); /* checks buffer capacity*/
int write_buffer(Buffer *buf, char *src, int len); /* adds to buffer */
void clear_buffer(Buffer *buf, int len); /* removes from buffer */
void free_buffer(Buffer *buf); /* frees buffer memory */
int dont_block(int fd); /* use nonblocking I/O */
int peers_has_space(Peers *peers, int need); /* makes sure peers has space*/
int add_peer(Peers *peers, int fd); /* adds a peer */
void remove_peer(Peers *peers, int i); /* removes peer */
void free_peers(Peers *peers); /* frees peer memory */
int timeout_val( char *s, int *timeout); /* parses timeout */
int host_port_vals(char *arg, char **host, char **port); /* parses host and port*/
int parse_args(int argc, char **argv, Arguments *cfg); /* parses command line arguments */
int make_listener( char *port); /* creates listener */
int connect_port(char *host, char *port); /* connects to port */
int send_peers(Peers *peers, int skip, char *buf, int len); /* sends set bytes to all peers */
int output_peers(Peers *peers, int i); /* writes output for peers */
int write_stdout(char *buf, int len); /* writes to stdout */
long long curr_time(void); /* returns monotonic time in ms */
int create_buffer_message(Buffer *dst, char *username, char *line, int len); /* formats buffer msg */
int process_stdin(Peers *peers, Buffer *input, char *username); /* processes raw input */
int handle_remaining(Peers *peers, Buffer *input, char *username); /* processes leftover before eof */



int main(int argc, char **argv) {
    Arguments args;
    int listener;
    int address;
    struct peers peers;
    struct buffer stdin_buf;
    int stdin_open;
    long long last_activity;

    signal(SIGPIPE, SIG_IGN);

    if (parse_args(argc, argv, &args) < 0) {
        return error_out(argv[0]);
    }

    listener = make_listener((char *)args.local_port);
    if (listener < 0) {
        return EXIT_FAILURE;
    }

    /* initialize structs */
    peers.data = NULL;
    peers.len = 0;
    peers.cap = 0;

    stdin_buf.data = NULL;
    stdin_buf.len = 0;
    stdin_buf.cap = 0;
    stdin_open = 1;

    if (dont_block(STDIN_FILENO) < 0) {
        perror("fcntl");
        close(listener);
        return EXIT_FAILURE;
    }

    if (args.remote_host != NULL) {
        address = connect_port((char *)args.remote_host, (char *)args.remote_port);
        if (address < 0) {
            close(listener);
            return EXIT_FAILURE;
        }

        if (add_peer(&peers, address) < 0) {
            perror("realloc");
            close(address);
            close(listener);
            return EXIT_FAILURE;
        }
    }

    last_activity = curr_time();

    while (1) {
        struct pollfd *fds;
        int num_descriptors;
        int i;
        int j;
        int timeout;
        int ready;

        num_descriptors = 1 + (stdin_open ? 1 : 0) + (int)peers.len;
        fds = malloc(num_descriptors * sizeof(*fds));
        if (fds == NULL) {
            perror("malloc");
            break;
        }

        i = 0;
        fds[i].fd = listener;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
        i++;

        if (stdin_open) {
            fds[i].fd = STDIN_FILENO;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
            i++;
        }

        for (j = 0; j < (int)peers.len; j++) {
            fds[i].fd = peers.data[j].fd;
            fds[i].events = POLLIN;
            if (peers.data[j].out.len > 0) {
                fds[i].events |= POLLOUT;
            }
            fds[i].revents = 0;
            i++;
        }

        if (args.timeout < 0) {
            timeout = -1;
        }
        else {
            long long elapsed = curr_time() - last_activity;
            long long remain = (long long)args.timeout - elapsed;

            if (remain <= 0) {
                free(fds);
                break;
            }

            timeout = (int)remain;
        }

        ready = poll(fds, (nfds_t)num_descriptors, timeout);
        if (ready < 0) {
            if (errno == EINTR) {
                free(fds);
                continue;
            }
            perror("poll");
            free(fds);
            break;
        }

        if (ready == 0) {
            free(fds);
            break;
        }

        i = 0;

        if (fds[i].revents & POLLIN) {
            for (;;) {
                int fd;

                fd = accept(listener, NULL, NULL);
                if (fd >= 0) {
                    if (dont_block(fd) < 0) {
                        close(fd);
                        continue;
                    }

                    if (add_peer(&peers, fd) < 0) {
                        perror("realloc");
                        close(fd);
                        free(fds);
                        goto cleanup_failure;
                    }
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                else {
                    break;
                }
            }
        }
        i++;

        if (stdin_open) {
            if (fds[i].revents & POLLIN) {
                char read_buf[READ_SIZE];
                int n;

                for (;;) {
                    n = read(STDIN_FILENO, read_buf, sizeof(read_buf));
                    if (n > 0) {
                        last_activity = curr_time();

                        if (write_buffer(&stdin_buf, read_buf, (int)n) < 0 ||
                            process_stdin(&peers, &stdin_buf,
                                (char *)args.username) < 0) {
                            perror("memory");
                            free(fds);
                            goto cleanup_failure;
                        }
                    }
                    else if (n == 0) {
                        if (handle_remaining(&peers, &stdin_buf,
                                (char *)args.username) < 0) {
                            perror("memory");
                            free(fds);
                            goto cleanup_failure;
                        }
                        stdin_open = 0;
                        break;
                    }
                    else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }
                    else if (errno == EINTR) {
                        continue;
                    }
                    else {
                        stdin_open = 0;
                        break;
                    }
                }
            }

            if (fds[i].revents & (POLLHUP | POLLNVAL)) {
                if (handle_remaining(&peers, &stdin_buf, (char *)args.username) < 0) {
                    perror("memory");
                    free(fds);
                    goto cleanup_failure;
                }
                stdin_open = 0;
            }

            i++;
        }

        for (j = 0; j < (int)peers.len; ) {
            int revents;
            int should_remove;

            revents = fds[i].revents;
            should_remove = 0;

            if (revents & POLLIN) {
                char read_buf[READ_SIZE];
                int n;

                for (;;) {
                    n = read(peers.data[j].fd, read_buf, sizeof(read_buf));
                    if (n > 0) {
                        last_activity = curr_time();

                        if (write_stdout(read_buf, (int)n) < 0) {
                            free(fds);
                            goto cleanup_failure;
                        }

                        if (send_peers(&peers, j, read_buf, (int)n) < 0) {
                            perror("memory");
                            free(fds);
                            goto cleanup_failure;
                        }
                    }
                    else if (n == 0) {
                        should_remove = 1;
                        break;
                    }
                    else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }
                    else if (errno == EINTR) {
                        continue;
                    }
                    else {
                        should_remove = 1;
                        break;
                    }
                }
            }

            if (!should_remove && (revents & POLLOUT)) {
                if (output_peers(&peers, j) < 0) {
                    should_remove = 1;
                }
            }

            if (revents & (POLLHUP | POLLERR | POLLNVAL)) {
                should_remove = 1;
            }

            if (should_remove) {
                remove_peer(&peers, j);
            }
            else {
                j++;
                i++;
            }
        }

        free(fds);
    }

    close(listener);
    free_buffer(&stdin_buf);
    free_peers(&peers);
    return EXIT_SUCCESS;

cleanup_failure:
    close(listener);
    free_buffer(&stdin_buf);
    free_peers(&peers);
    return EXIT_FAILURE;
}


int error_out(char *prog) {
    fprintf(stderr,
        "usage: %s [-u USERNAME] [-t TIMEOUT] PORT [HOSTNAME:PORT]\n",
        prog);
    return EXIT_FAILURE;
}

int buffer_cap(Buffer *buf, int need) {
    int cap;
    char *tmp;

    if (need <= (int)buf->cap) {
        return 0;
    }

    cap = buf->cap == 0 ? 64 : (int)buf->cap;
    while (cap < need) {
        if (cap > (int)(SIZE_MAX / 2)) {
            cap = need;
            break;
        }
        cap *= 2;
    }

    tmp = realloc(buf->data, cap);
    if (tmp == NULL) {
        return -1;
    }

    buf->data = tmp;
    buf->cap = cap;
    return 0;
}

int write_buffer(Buffer *buf, char *src, int len) {
    if (len == 0) {
        return 0;
    }

    if (buffer_cap(buf, (int)buf->len + len) < 0) {
        return -1;
    }

    memcpy(buf->data + buf->len, src, len);
    buf->len += len;
    return 0;
}

void clear_buffer(Buffer *buf, int len) {
    if (len >= (int)buf->len) {
         buf->len = 0;
        return;
    }

    memmove(buf->data, buf->data + len, buf->len - len);
    buf->len -= len;
}

void free_buffer(Buffer *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

int dont_block(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }

    return 0;
}

int peers_has_space(Peers *peers, int need) {
    int cap;
    Peer *tmp;

    if (need <= (int)peers->cap) {
        return 0;
    }

    cap = peers->cap == 0 ? 8 : (int)peers->cap;
    while (cap < need) {
        if (cap > (int)(SIZE_MAX / 2 / sizeof(*tmp))) {
            cap = need;
            break;
        }
        cap *= 2;
    }

    tmp = realloc(peers->data, cap * sizeof(*tmp));
    if (tmp == NULL) {
        return -1;
    }

    peers->data = tmp;
    peers->cap = cap;
    return 0;
}

int add_peer(Peers *peers, int fd) {
    if (peers_has_space(peers, (int)peers->len + 1) < 0) {
        return -1;
    }

    peers->data[peers->len].fd = fd;
    peers->data[peers->len].out.data = NULL;
    peers->data[peers->len].out.len = 0;
    peers->data[peers->len].out.cap = 0;
    peers->len++;
    return 0;
}

void remove_peer(Peers *peers, int i) {
    close(peers->data[i].fd);
    free_buffer(&peers->data[i].out);

    if ((i + 1) < peers->len) {
        peers->data[i] = peers->data[peers->len - 1];
    }
    peers->len--;
}

void free_peers(Peers *peers) {
    int i;

    for (i = 0; i < (int)peers->len; i++) {
        close(peers->data[i].fd);
        free_buffer(&peers->data[i].out);
    }

    free(peers->data);
    peers->data = NULL;
    peers->len = 0;
    peers->cap = 0;
}

int timeout_val(char *s, int *timeout) {
    char *end;
    long value;

    errno = 0;
    value = strtol(s, &end, 10);
    if (errno != 0 || *s == '\0' || *end != '\0') {
        return -1;
    }
    if (value < 0 || value > INT_MAX) {
        return -1;
    }

    *timeout = (int)value;
    return 0;
}

int host_port_vals(char *arg, char **host, char **port) {
    char *colon;

    colon = strrchr(arg, ':');
    if (colon == NULL || colon == arg || colon[1] == '\0') {
        return -1;
    }

    *colon = '\0';
    *host = arg;
    *port = colon + 1;
    return 0;
}

int parse_args(int argc, char **argv, Arguments *cfg) {
    int i;

    cfg->prog = argv[0];
    cfg->username = "nobody";
    cfg->timeout = -1;
    cfg->local_port = NULL;
    cfg->remote_host = NULL;
    cfg->remote_port = NULL;

    i = 1;
    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-u") == 0) {
            if (i + 1 >= argc) {
                return -1;
            }
            cfg->username = argv[i + 1];
            i += 2;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 >= argc || timeout_val(argv[i + 1], &cfg->timeout) < 0) {
                return -1;
            }
            i += 2;
        }
        else {
            return -1;
        }
    }

    if (i >= argc) {
        return -1;
    }

    cfg->local_port = argv[i];
    i++;

    if (i < argc) {
        if (host_port_vals(argv[i], &cfg->remote_host, &cfg->remote_port) < 0) {
            return -1;
        }
        i++;
    }

    if (i != argc) {
        return -1;
    }

    return 0;
}

int make_listener(char *port) {
    struct addrinfo hints;
    struct addrinfo *addr;
    struct addrinfo *it;
    int status;
    int fd;
    int yes;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &addr);
    if (status != 0) {
        fprintf(stderr, "%s\n", gai_strerror(status));
        return -1;
    }

    fd = -1;
    for (it = addr; it != NULL; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0) {
            continue;
        }

        yes = 1;
        (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(fd, it->ai_addr, it->ai_addrlen) == 0) {
            if (listen(fd, LISTEN_LIMIT) == 0) {
                break;
            }
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(addr);

    if (fd < 0) {
        fprintf(stderr, "%s: %s\n", port, strerror(errno));
        return -1;
    }

    if (dont_block(fd) < 0) {
        perror("fcntl");
        close(fd);
        return -1;
    }

    return fd;
}

int connect_port(char *host, char *port) {
    struct addrinfo hints;
    struct addrinfo *addr;
    struct addrinfo *it;
    int status;
    int fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, port, &hints, &addr);
    if (status != 0) {
        fprintf(stderr, "%s: %s\n", host, gai_strerror(status));
        return -1;
    }

    fd = -1;
    for (it = addr; it != NULL; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0) {
            continue;
        }

        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0) {
            break;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(addr);

    if (fd < 0) {
        perror("connect");
        return -1;
    }

    if (dont_block(fd) < 0) {
        perror("fcntl");
        close(fd);
        return -1;
    }

    return fd;
}

int send_peers(Peers *peers, int skip, char *buf, int len) {
    int i;

    for (i = 0; i < (int)peers->len; i++) {
        if (i == skip) {
            continue;
        }

        if (write_buffer(&peers->data[i].out, buf, len) < 0) {
            return -1;
        }
    }

    return 0;
}

int output_peers(Peers *peers, int i) {
    int n;

    while (peers->data[i].out.len > 0) {
        n = write(peers->data[i].fd,
            peers->data[i].out.data, peers->data[i].out.len);

        if (n > 0) {
            clear_buffer(&peers->data[i].out, (int)n);
        }
        else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return 0;
        }
        else {
            return -1;
        }
    }

    return 0;
}

int write_stdout(char *buf, int len) {
    int off;
    int n;

    off = 0;
    while (off < len) {
        n = write(STDOUT_FILENO, buf + off, (len - off));
        if (n > 0) {
            off += (int)n;
        }
        else if (n < 0 && errno == EINTR) {
            continue;
        }
        else {
            return -1;
        }
    }

    return 0;
}

long long curr_time(void) {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        return 0;
    }

    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

int create_buffer_message(Buffer *dst, char *username, char *line, int len) {
    if (write_buffer(dst, username, (int)strlen(username)) < 0) {
        return -1;
    }
    if (write_buffer(dst, ": ", 2) < 0) {
        return -1;
    }
    if (write_buffer(dst, line, len) < 0) {
        return -1;
    }
    return 0;
}

int process_stdin(Peers *peers, Buffer *input, char *username) {
    int i;
    int start;
    Buffer line;

    start = 0;
    for (i = 0; i < (int)input->len; i++) {
        if (input->data[i] == '\n') {
            line.data = NULL;
            line.len = 0;
            line.cap = 0;

            if (create_buffer_message(&line, username,
                    input->data + start, i - start + 1) < 0) {
                free_buffer(&line);
                return -1;
            }

            if (send_peers(peers, -1, line.data, (int)line.len) < 0) {
                free_buffer(&line);
                return -1;
            }

            free_buffer(&line);
            start = i + 1;
        }
    }

    if (start > 0) {
        clear_buffer(input, start);
    }

    return 0;
}

int handle_remaining(Peers *peers, Buffer *input, char *username) {
    Buffer line;

    if (input->len == 0) {
        return 0;
    }

    line.data = NULL;
    line.len = 0;
    line.cap = 0;

    if (create_buffer_message(&line, username, input->data, (int)input->len) < 0) {
        free_buffer(&line);
        return -1;
    }

    if (send_peers(peers, -1, line.data, (int)line.len) < 0) {
        free_buffer(&line);
        return -1;
    }

    free_buffer(&line);
    input->len = 0;
    return 0;
}