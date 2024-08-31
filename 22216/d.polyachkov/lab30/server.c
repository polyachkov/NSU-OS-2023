#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/my_socket"

void to_uppercase(char *str) {
    for (char *p = str; *p; ++p) {
        *p = toupper((unsigned char)*p);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[256];

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
        perror("accept");
        close(server_fd);
        return -1;
    }

    while (1) {
        ssize_t len;
        if ((len = read(client_fd, buffer, sizeof(buffer) - 1)) <= 0) {
            if (len < 0) perror("read");
            break;
        }
        buffer[len] = '\0';

        to_uppercase(buffer);
        printf("Received: %s\n", buffer);
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}