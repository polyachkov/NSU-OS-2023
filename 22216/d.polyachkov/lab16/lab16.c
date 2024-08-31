#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void set_terminal_mode() {
    struct termios new_settings;
    if (tcgetattr(STDIN_FILENO, &new_settings) != 0) {
        perror("Error getting terminal attributes");
        exit(-1);
    }
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_settings) != 0) {
        perror("Error setting terminal attributes");
        exit(-1);
    }
}

void reset_terminal_mode(struct termios *original_settings) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, original_settings) != 0) {
        perror("Error restoring terminal attributes");
        exit(-1);
    }
}

int main() {
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Standard input is not a terminal.\n");
        return -1;
    }

    struct termios original_settings;
    if (tcgetattr(STDIN_FILENO, &original_settings) != 0) {
        perror("Error getting terminal attributes");
        return -1;
    }

    set_terminal_mode();

    printf("Continue? (y/n): ");
    fflush(stdout);

    char answer = getchar();
    if (answer == EOF) {
        perror("Error reading character");
        reset_terminal_mode(&original_settings);
        return -1;
    }

    reset_terminal_mode(&original_settings);

    printf("\n");

    if (answer == 'y' || answer == 'Y') {
        printf("You chose to continue.\n");
    } else if (answer == 'n' || answer == 'N') {
        printf("You chose to stop.\n");
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}
