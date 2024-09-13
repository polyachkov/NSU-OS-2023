#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 5

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

void cleanup(int signum) {
    unlink("/tmp/tcp_unix_socket");
    _exit(0);
}

int setup_server_socket(const char *socket_path) {
    int server_fd;
    struct sockaddr_un server_addr;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind() failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen() failed");
        close(server_fd);
        unlink(socket_path);
        exit(1);
    }

    return server_fd;
}

void handle_connections(int server_fd) {
    struct pollfd fds[MAX_CONNECTIONS + 1];
    int connection_count = 0;
    char buffer[BUFFER_SIZE + 1];

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        int poll_result = poll(fds, connection_count + 1, -1);
        if (poll_result < 0) {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("accept() failed");
            } else {
                fds[connection_count + 1].fd = client_fd;
                fds[connection_count + 1].events = POLLIN;
                connection_count++;
                if (connection_count == MAX_CONNECTIONS){
                    fds[0].events = 0;
                }
            }
        }

        for (int i = 1; i <= connection_count; i++) {
            if (fds[i].revents & POLLIN) {
                int read_len = read(fds[i].fd, buffer, BUFFER_SIZE);
                if (read_len > 0) {
                    buffer[read_len] = '\0';
                    to_uppercase(buffer);
                    printf("Received: %s\n", buffer);
                } else {
                    if (read_len < 0) {
                        perror("read() failed");
                    }
                    close(fds[i].fd);
                    fds[i] = fds[connection_count];
                    connection_count--;
                    fds[0].events = POLLIN;
                }
            }
        }
    }
}

int main() {
    const char *socket_path = "/tmp/tcp_unix_socket";
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int server_fd = setup_server_socket(socket_path);
    handle_connections(server_fd);

    close(server_fd);
    unlink(socket_path);
    return 0;
}
