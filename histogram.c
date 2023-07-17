#include <stdio.h>
#include "histogram.h"
#include <string.h>
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