#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        char *args[] = {"cat", argv[1], NULL};
        execvp("cat", args);
        perror("execvp failed");
        exit(1);
    }

    time_t start = time(NULL);
    if(start == (time_t) -1) {
        perror("time error");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        exit(1);
    }

    time_t end = time(NULL);
    if(end == (time_t) -1) {
        perror("time error");
        exit(1);
    }

    printf("Parent waited %ld seconds for child to complete.\n", end - start);

    if (WIFEXITED(status)) {
        printf("Child exited normally with status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Child was terminated by signal: %d\n", WTERMSIG(status));
    }

    return 0;
}
