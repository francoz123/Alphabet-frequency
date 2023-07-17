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
int make_trivial_ring(int [2]);
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
    get_file_array(dirname, file_names, file_count);
    
    int fds[num_processes][2];
    int pid = getpid();
    int p_number = 0;
    int childpid;
    int counts[26] = {0};

    for (int i = 0; i < num_processes; i++) {
        if(make_trivial_ring(fds[i]) < 0){
            perror("Failed trivial ring");
            exit(EXIT_FAILURE); 
        }
    }

    for (int i = 0; i < num_processes-1;  i++) {
        if(add_new_node(&childpid) < 0){
            perror("Could not add new node to ring");
            exit(EXIT_FAILURE); }; 
        if (childpid == 0){
            int start_index = i * file_count / num_processes;;
            int end_index = (i + 1) * file_count / num_processes;
            process_files(file_names, start_index, end_index, counts);
            int result[26] = {0};
            read(fds[i][0], result, sizeof(result));
            close(fds[i][0]);
            for (int j = 0; j < 26; j++) {
                counts[j] += result[j];
            }
            
            write(fds[i+1][1], counts, sizeof(counts));

            if (i+1 < num_processes-1) close(fds[i+1][1]);

            for (int i = 0; i < file_count; i++) {
                free(file_names[i]);
            }

            free(file_names);
            exit(0); 
        }
    }


    for (int i = 1; i < num_processes; i++) {
        close(fds[i][1]);
    }
        
    int start_index = (num_processes - 1) * file_count / num_processes;;
    int end_index = num_processes;
    
    process_files(file_names, start_index, end_index, counts);
 
    write(fds[0][1], counts, sizeof(counts));
    close(fds[0][1]);
    for (int i = 0; i < num_processes-1; i++) { 
        wait(NULL);
    }
    int result[26] = {0};
    read(fds[num_processes-1][0], result, sizeof(result));
    close(fds[num_processes-1][1]);
    
    histogram(result, 100, '*', 26);
    for (int i = 0; i < file_count; i++) {
        free(file_names[i]);
    }
    free(file_names);
    //exit(0);
    return 0;
}

int add_new_node(int *pid){
    if ((*pid = fork()) == -1) 
        return(-1); 
    return(0);
}

void process_files(char *file_names[], int start_index, int last_index, int frequencies[26]) {
    for (int i = start_index; i < last_index; i++) {
        FILE *file = fopen(file_names[i], "r");
        if (file == NULL) {
            perror("Error encountered");
            exit(1);
        }

        int c;
        while ((c = fgetc(file)) != EOF) {
            if((tolower(c)-'a')>=0 && (tolower(c)-'a') < 26){
                frequencies[tolower(c)-'a']++;
            }
        }

        fclose(file);
    }
}

int parse_args(int argc,  char *argv[], int *np){
    if ( (argc != 3) || ((*np = atoi (argv[1])) <= 0) ) {
        fprintf (stderr, "Usage: %s [directory name]\n", argv[0]);
        return(-1); };
    return(0); 
}

int make_trivial_ring(int *fd){   
    if (pipe (fd) == -1) 
        return(-1); 
    return(0); 
}

int count_files(const char *dirname)
{
    DIR *dir;
    struct dirent *f;
    dir = opendir(dirname);

    if (dir == NULL){
        perror("Error occured while trying to open the specified directory\n");
        exit(2);
    }

    int fc = 0;
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_type == DT_REG)
        {
            fc++;
        }
    }
    closedir(dir);
    return fc;
}

void get_file_array(const char *dirname, char **files, int n)
{
    DIR *dir;
    struct dirent *f;
    dir = opendir(dirname);

    for (int i = 0; i < n; i++)
    {
        files[i] = (char*)malloc(sizeof(f->d_name) * sizeof(char));
    }
    if (dir == NULL){
        perror("Error occured while trying to open the specified directory\n");
        exit(2);
    }
    int i = 0;
    while ((f = readdir(dir)) != NULL && i < n)
    {
        if (f->d_type == DT_REG)
        {
            snprintf(files[i++], 26, "%s/%s", dirname, f->d_name);
        }
    }
    closedir(dir);
}

void get_file_array2(const char *dirname, char ***files, int n)
{
    DIR *dir;
    struct dirent *f;
    dir = opendir(dirname);
    char **fnames = malloc(n * sizeof(char*));

    for (int i = 0; i < n; i++)
    {
        fnames[i] = (char*)malloc(sizeof(f->d_name) * sizeof(char));
    }
    if (dir == NULL){
        perror("Error occured while trying to open the specified directory\n");
        exit(2);
    }
    int i = 0;
    while ((f = readdir(dir)) != NULL && i < n)
    {
        if (f->d_type == DT_REG)
        {
            snprintf(fnames[i++], 26, "%s/%s", dirname, f->d_name);
        }
    }
    closedir(dir);
    *files = fnames;
}

void histogram(int *array, int max_bar_length, char c, int array_size){
    int total = 0;
    int max_count = 0;
    for (int i = 0; i < array_size; i++) { 
        if (array[i] > max_count) max_count = array[i];
        total += array[i];
    }
    
    int max_copy = max_count;
    int max_char_length = 1;
    while (max_copy/10 > 0){
        max_copy = max_copy/10;
        max_char_length++;
    }
    
    for (int i = 0; i < array_size; i++) { 
        int bar_length = (((double)array[i]/max_count) * max_bar_length);
        if (bar_length == 0) bar_length = 1;
        char buf[max_char_length + 1];
        snprintf(buf, max_char_length + 1, "%d", array[i]);
        int len = strlen(buf);
        memset(buf + len, ' ', (max_char_length - len) * sizeof(char));

        char hist[max_bar_length + 1];
        memset(hist, c, bar_length * sizeof(char));
        hist[bar_length] = '\0';
        printf("Process 1 got char %c: %s | %s\n", 1+'a', buf, hist);
    }
    
}