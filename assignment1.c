/**
 * This program calculates and prints a histogram of frequencies
 * of occurence of the characters a-z in a directory, ignoring case.
 * Computation can be parallelized between two or more processes for efficiency.
 * 
 * Usage: 
 *      Compile program: make
 *      Run program: assignment1 number_of_processes directory_name
 * Example: `assignment1 3 myfiles`
 * 
 * @author Francis Ozoka
 * COSC330 Assigment 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "ring_of_process.h"
#include "histogram.h"
#include "file_operations.h"

int parse_args(int,  char **, int*);


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




