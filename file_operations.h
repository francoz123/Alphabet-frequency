#ifndef FILE_OPERATIONS
#define FILE_OPERATIONS

int count_files(const char*);
void get_file_array(const char*, char**, int);
void get_file_array2(const char*, char***, int);
void process_files(char *[], int, int, int [26]);

#endif