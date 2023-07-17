#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 256

void process_files(char *filenames[], int start, int end, int counts[26]) {
    for (int i = start; i < end; i++) {
        FILE *file = fopen(filenames[i], "r");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        int c;
        while ((c = fgetc(file)) != EOF) {
            if((tolower(c)-'a')>=0 && (tolower(c)-'a') < 26){
                counts[tolower(c)-'a']++;
            }
        }

        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s NUM_PROCESSES DIRECTORY\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_processes = atoi(argv[1]);
    char *directory = argv[2];

    DIR *dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    char *filenames[MAX_FILES];
    int num_files = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            filenames[num_files] = malloc(MAX_FILENAME_LENGTH);
            snprintf(filenames[num_files], MAX_FILENAME_LENGTH, "%s/%s", directory, entry->d_name);
            num_files++;
        }
    }

    closedir(dir);

    int pipefds[num_processes][2];

    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(pipefds[i][0]);

            int start = i * num_files / num_processes;
            int end = (i + 1) * num_files / num_processes;

            int counts[256] = {0};
            process_files(filenames, start, end, counts);

            write(pipefds[i][1], counts, sizeof(counts));
            close(pipefds[i][1]);

            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        close(pipefds[i][1]);
    }

    int total_counts[256] = {0};

    for (int i = 0; i < num_processes; i++) {
        int counts[256];
        read(pipefds[i][0], counts, sizeof(counts));
        close(pipefds[i][0]);

        for (int j = 0; j < 256; j++) {
            total_counts[j] += counts[j];
        }
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    for (int i = 0; i < 256; i++) {
        if (total_counts[i] > 0) {
            printf("%c: %d\n", i, total_counts[i]);
        }
    }

    for (int i = 0; i < num_files; i++) {
        free(filenames[i]);
    }

    return 0;
}
