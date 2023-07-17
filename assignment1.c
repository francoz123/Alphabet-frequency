/**
 * This program calculates and prints a histogram of frequencies
 * of occurence of the characters a-z in a directory, ignoring case.
 * Computation can be parallelized between two or more processes for efficiency.
 * 
 * Usage: assignment1 number_of_processes directory_name
 * 
 * @author Francis Ozoka
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

int count_files(const char*);
void get_file_array(const char*, char**, int);
void get_file_array2(const char*, char***, int);
int make_trivial_ring();
int make_trivial_ring(int*);
int add_new_node(int*);
int parse_args(int,  char **, int*);
void process_files(char *[], int, int, int [26]);
void histogram(int*, int, char, int);


int main(int argc, char *argv[])
{
    int num_processes;
    if (parse_args(argc,  argv, &num_processes) < 0) exit(1);
    
    char *dirname = argv[2];
    int file_count = count_files(dirname);
    char **file_names = malloc(file_count * sizeof(char*));
    get_file_array(dirname, file_names, file_count); // Populate array of file names
    
    int childpid; // Holds process IDs of newly created child process
    int counts[26] = {0}; // Holds character counts
    int fd[2];
    int main_pid = getpid(); // Main process pid for crontrolling information flow

    // Initial pipe for ring-of-process communication
    if (make_trivial_ring(fd) < 0){
        perror("Unable to initiate ring of process");
        exit(1);
    }
    int p_number = num_processes-1; // unique number for each process
    
    // Create ring of process
    for (int i = 0; i < num_processes-1;  i++) {
        if(add_new_node(&childpid) < 0){
            perror("Could not add new node to ring");
            exit(1); 
        }
        if (childpid) break;
        p_number = i; // Set process number
    }
        
    // Close unused pipes
    if (p_number < num_processes-2){
        if ((close(fd[0]) == -1) || (close(fd[1]) == -1)){
            perror("Attempt to close pipe fialed");
            exit(1);
        }
    }else if (p_number == num_processes-2){
        /**
         * Last process in the ring
         * Redirect output to the main process
        */
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("Failes to duplicate pipe");
            exit(1);
        }
        if ((close(fd[0]) == -1) || (close(fd[1]) == -1)){
            perror("Attempt to close pipe fialed");
            exit(1);
        }
    }
    
    // Each process calcuactes it's range of sub-files based on its starting index
    int start_index = p_number * file_count / num_processes;
    int end_index = (p_number + 1) * file_count / num_processes;

    process_files(file_names, start_index, end_index, counts);

    int result[26] = {0}; // Used for readind in results from preseeding process
    int n = sizeof(counts);
    if (getpid() == main_pid){ // Main process
        if (write(STDOUT_FILENO, counts, sizeof(counts)) < n){
            perror("Write error");
            exit(1);
        }
    }else{
        // Child processes
        if ((n = read(STDIN_FILENO, result, sizeof(result)) < 0)){
            perror("Read error");
            exit(2);
        }
        // Sum counts
        for (int j = 0; j < 26; j++) {
            counts[j] += result[j];
        }
        if (write(STDOUT_FILENO, counts, sizeof(counts)) != n){
            perror("Write earror");
            exit(3);
        }
        // Free allocated memory
        for (int i = 0; i < file_count; i++) {
            free(file_names[i]);
        } 

        free(file_names);
        exit(0); // Child processes exits here
    }
    // Read resullt from last process in the ring
    if (read(fd[0], result, sizeof(result)) < 0){
        perror("Write earror");
        exit(1);
    }
    // Close unnede pipes 
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1)){
        perror("Attempt to close pipe fialed");
        exit(1);
    }
    // Display histogram
    histogram(result, 100, '*', 26);
    // Free allocated memory
    for (int i = 0; i < file_count; i++) {
        free(file_names[i]);
    }

    free(file_names);
    return 0;
}

/**
 * Adds a new node to the ring of process
 * @param pid - int pointer for storing the pid of the child process
 * @return int
*/
int add_new_node(int *pid){
    int fd[2]; 
    if (pipe(fd) == -1) 
        return(-1); 
    if ((*pid = fork()) == -1)
        return(-2); 
    // stdio redirection
    if(*pid > 0 && dup2(fd[1], STDOUT_FILENO) < 0)
        return(-3); 
    if (*pid == 0 && dup2(fd[0], STDIN_FILENO) < 0)
        return(-4); 
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) 
        return(-5);
    return(0);
}

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
 * Parses and and retrievs CL arguments
 * @param argc
 * @param argc
 * @param np int pointer to store number of processes
 * @return int Error code
*/
int parse_args(int argc,  char *argv[], int *np){
    if (argc != 3) {
        fprintf (stderr, "Usage: %s [directory name]\n", argv[0]);
        return(-1); 
    }

    if (((*np = atoi (argv[1])) <= 0)) {
        fprintf (stderr, "Number of processes must be at least one");
        return(-1); 
    };

    return(0); 
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

/**
 * Prints histogram of character frequencies
 * @param array Pointer to array of character frequencies
 * @param max_bar_length
 * @param c character tobe used in printing the histogram
 * @param array_size size of the array
 * @return void
*/
void histogram(int *array, int max_bar_length, char c, int array_size){
    int total = 0;
    int max_count = 0;
    // Get the maximum frequency and calculate total
    for (int i = 0; i < 26; i++) { 
        if (array[i] > max_count) max_count = array[i];
        total += array[i];
    }
    
    // Calculate number of character needed
    int max_copy = max_count;
    int max_char_length = 1;

    while (max_copy/10 > 0){
        max_copy = max_copy/10;
        max_char_length++;
    }
    
    // Normalize frequencies and padd output accordingly
    for (int i = 0; i < 26; i++) { 

        int bar_length = (((double)array[i]/max_count) * max_bar_length);

        if (bar_length == 0) bar_length = 1;

        char buff[max_char_length + 1];
        memset(buff, 0, sizeof(buff)); // Initialize buff with zeros
        snprintf(buff, max_char_length + 1, "%d", array[i]);
        int len = strlen(buff);
        // Pad buff with spaces for allignment
        memset(buff + len, ' ', (max_char_length - len) * sizeof(char));
        char hist[max_bar_length + 1];
        memset(hist, c, bar_length * sizeof(char));
        hist[bar_length] = '\0';
        fprintf(stderr,"Process 1 got char %c: %s | %s\n", i+'a', buff, hist);
    }
    
}

/**
 * Creates initial ring for ring of process
 * @param fd Pointer to file descriptors
*/
int make_trivial_ring(int *fd){   
    if (pipe (fd) == -1) 
        return(-1); 
    if ((dup2(fd[1], STDOUT_FILENO) == -1) ||
    (dup2(fd[0], STDIN_FILENO) == -1)) 
        return(-2); 
    return(0); 
}