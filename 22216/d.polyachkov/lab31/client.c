#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

void connect_to_server(int *client_fd, const char *socket_path, struct sockaddr_un *server_addr) {
    *client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*client_fd < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(server_addr, 0, sizeof(struct sockaddr_un));
    server_addr->sun_family = AF_UNIX;
    strncpy(server_addr->sun_path, socket_path, sizeof(server_addr->sun_path) - 1);

    if (connect(*client_fd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_un)) < 0) {
        perror("connect() failed");
        close(*client_fd);
        exit(1);
    }
}

void send_data(int client_fd) {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (write(client_fd, buffer, strlen(buffer)) < 0) {
            perror("write() failed");
            break;
        }
    }
}

int main() {
    const char *socket_path = "/tmp/tcp_unix_socket";
    int client_fd;
    struct sockaddr_un server_addr;

    connect_to_server(&client_fd, socket_path, &server_addr);
    send_data(client_fd);

    close(client_fd);
    return 0;
}
