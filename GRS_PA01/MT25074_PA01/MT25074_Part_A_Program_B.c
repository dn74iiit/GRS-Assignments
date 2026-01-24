#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include <string.h>
#include <unistd.h>
#include "MT25074_Part_B_Program.h"

//wrapper for thread workers because cant pass cpu directly return types aren't void
void *cpu_wrapper(void * args)
{
    long long limit = (long long)args;
    pthread_t current_thread_id = pthread_self();
    printf("Inside the thread with ID - %lu\n", (unsigned long)current_thread_id);

    cpu(limit);
    return NULL;
}
void *mem_wrapper(void * args)
{
    long long limit = (long long)args;
    pthread_t current_thread_id = pthread_self();
    printf("Inside the thread with ID - %lu\n", (unsigned long)current_thread_id);

    mem(limit);
    return NULL;
}
void *io_wrapper(void * args)
{
    long long limit = (long long)args;
    pthread_t current_thread_id = pthread_self();
    printf("Inside the thread with ID - %lu\n", (unsigned long)current_thread_id);

    io(limit);
    return NULL;
}


int main(int argc, char *argv[]) {

    char *task;
    int num_threads;

    //default cases of user inputs
    if (argc == 2) {
        printf("Usage: %s <task> <threads_count>\n", argv[0]);
        task = argv[1];
        num_threads = 2;
    }
    else if (argc == 1)
    {
        printf("Usage: %s <task> <threads_count>\n", argv[0]);
        task = "cpu";
        num_threads = 2;
    }
    else
    {
        task = argv[1];
        num_threads = atoi(argv[2]);
    }



    long long roll_cycles = 8000;

    //variable to store wrapper worker function pointer and assign according to cmd args
    void *(*wrapper_worker)(void *);

    if (strcmp(task, "cpu") == 0) {
        wrapper_worker = cpu_wrapper;
    }
    else if (strcmp(task, "mem") == 0) {
        wrapper_worker = mem_wrapper;
    }
    else if (strcmp(task, "io") == 0) {
        wrapper_worker = io_wrapper;
    }
    else {
        fprintf(stderr, "Error: Unknown task '%s'\n", task);
        return 1;
    }

    printf("creating threads - %d\n", num_threads);
    //array to hold threads
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        // We cast limit to (void*) to pass it through the generic argument
        if (pthread_create(&threads[i], NULL, wrapper_worker, (void *)roll_cycles) != 0) {
            perror("Failed to create thread");
            free(threads);
            return 1;
        }

    }
    //waiting for all the threads to finish execution
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%d threads completed execution successfully.\n", num_threads);

    //freeing the thread array
    free(threads);

    return 0;
}