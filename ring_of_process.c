#include "ring_of_process.h"
#include <unistd.h>
#include <stdlib.h>

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