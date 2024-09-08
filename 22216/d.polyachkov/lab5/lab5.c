#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define START_SIZE 50

typedef struct {
    off_t *offsets;
    off_t *lengths;
    int size;
    int capacity;
} DynamArr;

void freedynamArr(DynamArr *dynamArr) {
    free(dynamArr->offsets);
    free(dynamArr->lengths);
    dynamArr->size = 0;
    dynamArr->capacity = 0;
}

int initdynamArr(DynamArr *dynamArr) {
    dynamArr->offsets = (off_t *) malloc(START_SIZE * sizeof(off_t));
    if (dynamArr->offsets == NULL) {
        perror("Error malloc");
        return -1;
    }
    dynamArr->lengths = (off_t *) malloc(START_SIZE * sizeof(off_t));
    if (dynamArr->lengths == NULL) {
        perror("Error malloc");
        free(dynamArr->offsets);
        return -1;
    }
    dynamArr->size = 0;
    dynamArr->capacity = START_SIZE;
    return 0;
}

int add(DynamArr *dynamArr, off_t offset, off_t length) {
    if (dynamArr->size == dynamArr->capacity) {
        dynamArr->capacity *= 2;

        off_t* newo = (off_t *) realloc(dynamArr->offsets, dynamArr->capacity * sizeof(off_t));
        if (newo == NULL) {
            perror("Error realloc");
            return -1;
        }
        dynamArr->offsets = newo;

        off_t* newl = (off_t *) realloc(dynamArr->lengths, dynamArr->capacity * sizeof(off_t));
        if (newl == NULL) {
            perror("Error realloc");
            return -1;
        }
        dynamArr->lengths = newl;
    }
    dynamArr->offsets[dynamArr->size] = offset;
    dynamArr->lengths[dynamArr->size++] = length;
    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Incorrect usage.\nUsage: %s <filename>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    DynamArr table;
    if (initdynamArr(&table) < 0) {
        return -1;
    } 

    char buffer[BUFSIZ];
    ssize_t bytes_read;
    off_t prev_offset = 0;
    off_t curr_offset = 0;

    while ((bytes_read = read(fd, buffer, BUFSIZ)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            curr_offset++;
            if (buffer[i] == '\n') {
                if (add(&table, prev_offset, curr_offset - prev_offset) < 0) {
                    freedynamArr(&table);
                    return -1;
                }
                prev_offset = curr_offset;
            }
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        freedynamArr(&table);
        return -1;
    }

    if (add(&table, prev_offset, curr_offset - prev_offset) < 0) {
        freedynamArr(&table);
        return -1;
    }

    if (table.size == 0) {
        printf("File is empty. Nothing to do. End of the program.\n");
        return 0;
    }

    while (1) {
        int findLine = -1;
        printf("\nInput number of target line: ");
        int sc = scanf("%d", &findLine);
        
        if (feof(stdin)) { //sc == EOF - check for ctrl + D
            break;
        }
        if (sc == 1){
            if (findLine == 0){
                break;
            }
        }
        if (sc != 1 || findLine < 0 || findLine > table.size) {
            fprintf(stderr, "Incorrect line number. Line should be a number from 1 to %d.\n", table.size);
            char ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }
        findLine--;

        lseek(fd, table.offsets[findLine], SEEK_SET);

        char buffer_str[BUFSIZ];
        off_t bytes_left = table.lengths[findLine];

        while (bytes_left > 0) {
            off_t read_size = (bytes_left < BUFSIZ ? bytes_left : BUFSIZ);
            ssize_t bytes_read = read(fd, buffer_str, read_size);
            if (bytes_read <= 0) {
                if (bytes_read == -1) {
                    perror("Error reading file");
                    freedynamArr(&table);
                    return -1;
                }
                break;
            }
            bytes_left -= bytes_read;
            fwrite(buffer_str, sizeof(char), bytes_read, stdout);
            if (ferror(stdout)) { 
                fprintf(stderr, "Error writing to stdout");
                freedynamArr(&table);
                return -1;
            }
        }
    }

    close(fd);
    freedynamArr(&table);
    return 0;
}
