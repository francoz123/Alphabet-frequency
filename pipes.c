#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include<stdlib.h>

int make_trivial_ring(){   
    int   fd[2];
    if (pipe (fd) == -1) 
        return(-1); 
    if ((dup2(fd[0], STDIN_FILENO) == -1) ||
        (dup2(fd[1], STDOUT_FILENO) == -1)) 
        return(-2); 
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1))   
        return(-3); 
    return(0); 
}

int add_new_node(int *pid){
    int fd[2];
    if (pipe(fd) == -1) 
        return(-1); 
    if ((*pid = fork()) == -1)
        return(-2); 
    if(*pid > 0 && dup2(fd[1], STDOUT_FILENO) < 0)
        return(-3); 
    if (*pid == 0 && dup2(fd[0], STDIN_FILENO) < 0)
        return(-4); 
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) 
        return(-5);
    return(0);
}
int main(void) {
    make_trivial_ring();
    int c;
    int p;
    int a[12] = {0};
    int fd[2];
    pipe(fd);
    for (int i = 0; i < 3; i++)
    {
        add_new_node(&c);
        if (c > 0)
        {
            for (int i = 0; i < 12; i++)
            {
                a[i] = a[i]+1;
            }
            
            write(STDOUT_FILENO, a, sizeof(a));
            break;
        }else {
            char b[50];
            
            read(STDIN_FILENO, a, sizeof(a));
            for (int i = 0; i < 12; i++)
            {
                //sprintf(b, "%d",  a[i]);
                fprintf(stderr, "%d: ", a[i]);
                
            }
            fprintf(stderr, "\n");
            
        }
    }
    
    
}