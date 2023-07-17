#include "file_operations.h"
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>


/**
 * Opens a subset of files and computes it's character frequencies
 * @param file_names  Pointer to an array of file names
 * @param start_index Index to start from
 * @param last_index  Index to stop at. Excluded
 * @param frequencies Array to store character counts
 * @return void
*/
void process_files(char *file_names[], int start_index, int last_index, int frequencies[26]) {
    for (int i = start_index; i < last_index; i++) {
        FILE *file = fopen(file_names[i], "r");
        if (file == NULL) {
            perror("File error");
            exit(1);
        }

        int c;
        // Count each character
        while ((c = fgetc(file)) != EOF) {
            if((tolower(c)-'a')>=0 && (tolower(c)-'a') < 26){
                frequencies[tolower(c)-'a']++;
            }
        }

        fclose(file);
    }
}

/**
 * Counts number of files in a directory
 * @param dirname
 * @return int Number of files in the directory
*/
int count_files(const char *dirname){
    DIR *dir;
    struct dirent *f;
    dir = opendir(dirname);

    if (dir == NULL){
        perror("Error occured while trying to open the specified directory\n");
        exit(2);
    }

    int fc = 0;
    while ((f = readdir(dir)) != NULL){
        if (f->d_type == DT_REG){ // Regular file
            fc++;
        }
    }
    closedir(dir);
    return fc;
}

/**
 * Populates an arry with file paths
 * @param dirname
 * @param files File array pointer
 * @return void
*/
void get_file_array(const char *dirname, char **files, int n)
{
    DIR *dir;
    struct dirent *f;
    dir = opendir(dirname);

    for (int i = 0; i < n; i++){
        files[i] = (char*)malloc(sizeof(f->d_name) * sizeof(char)); // Allocate memory
    }
    if (dir == NULL){
        perror("Error occured while trying to open the specified directory\n");
        exit(2);
    }
    int i = 0;
    while ((f = readdir(dir)) != NULL && i < n){ // Read file names
        if (f->d_type == DT_REG){
            // Write file path to file array index
            sprintf(files[i++], /* 256,  */"%s/%s", dirname, f->d_name);
        }
    }
    closedir(dir);
}
