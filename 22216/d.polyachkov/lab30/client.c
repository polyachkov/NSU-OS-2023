#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/my_socket"

int main() {
    int sock;
    struct sockaddr_un addr;
    char message[256];

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    while (1) {
        printf("Enter message (type 'exit' to quit): ");
        if (fgets(message, sizeof(message), stdin) == NULL) {
            perror("fgets");
            close(sock);
            return -1;
        }
        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "exit") == 0) {
            break;
        }

        if (write(sock, message, strlen(message)) < 0) {
            perror("write");
            close(sock);
            return -1;
        }
    }

    close(sock);
    return 0;
}