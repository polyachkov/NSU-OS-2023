#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s Erorr in usage. Please type COMMAND [arguments]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        return 1;
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed");
            return 1;
        }

        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process was terminated by signal %d\n", WTERMSIG(status));
        } else {
            printf("Child process did not exit normally\n");
        }
    }

    return 0;
}
