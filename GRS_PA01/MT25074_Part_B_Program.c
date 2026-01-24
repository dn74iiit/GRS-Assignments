#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include "MT25074_Part_B_Program.h"

// ---------------------------------------------------------
// CPU TASK

void cpu(long long input) {
    long long cycles = input * 15000LL;
    volatile double val = 0.0;

   //simple trigonometry functions computation
    for (long long i = 0; i < cycles; i++) {
       val += sin(i) * cos(i);
    }
}

// ---------------------------------------------------------
// MEMORY TASK

void mem(long long input) {

    long long arr_size = 64000000; // 256 MB

   //large array allocation
    int *data_array = (int *)malloc(arr_size * sizeof(int));

    if (data_array == NULL) {
       printf("Memory allocation failed!\n");
       return;
    }

   //initialise array to zeroes
    memset(data_array, 0, arr_size * sizeof(int));

    //used volatile to make sure cpu instructions are done regularly
    volatile int sink = 0;

    //long limit to increase cycles
    long long limit = input * 25000LL;

    //memory access work
    for (long long i = 0; i < limit; i++) {

       //gets the data from RAM and updates
       data_array[i % arr_size] += 1;
       sink = data_array[i % arr_size];
    }

    (void)sink;
    //freeing up the memory
    free(data_array);
}

// ---------------------------------------------------------
// IO TASK

void io(long long input) {

    char fname[100];
    char temp[4096]; // 4KB buffer
    //initialising the memory
    memset(temp, 'A', sizeof(temp));

    unsigned long tid = (unsigned long)pthread_self();
    //  using /tmp/ to use the disk
    snprintf(fname, sizeof(fname), "/tmp/MT25074_IO_P%d_T%lu.txt", getpid(), tid);

    FILE *filePointer = fopen(fname, "w");
    if (filePointer == NULL) return;


    // 80,000 * 4KB = 320 MB Total Write.
    long long adjusted_input = input * 10;

    for (long long i = 0; i < adjusted_input; i++) {
       fwrite(temp, 1, sizeof(temp), filePointer);

       // Sync less often (Every 1000 instead of 500)
       // This allows the OS to buffer more, increasing the KB/s speed
       // seen by iostat when it finally flushes.
       if (i % 1000 == 0) {
          fsync(fileno(filePointer));
       }
    }
    //close and delete the file
    fclose(filePointer);
    unlink(fname);
}