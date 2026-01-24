#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<string.h>
#include"MT25074_Part_B_Program.h"

int main(int argc, char *argv[]) {


    char *task;
    int processes_count;

    //checking argument count for cmdline inputs
    if (argc == 2) {
        printf("Usage: %s <task> <processess_count>\n", argv[0]);
        task = argv[1];
        processes_count = 2;
    }
    else if (argc == 1)
    {
        printf("Usage: %s <task> <processess_count>\n", argv[0]);
        task = "cpu";
        processes_count = 2;
    }
    else
    {
        task = argv[1];
        processes_count = atoi(argv[2]);
    }

    //roll cycles are increased to get better stats
    long long roll_cycles = 8000; //4*10^3



    printf("hello we are currently parent process with (pid:%d)\n", (int) getpid());
    printf("Creating %d processes....\n", processes_count);

    //Creating processes
    for (int i = 0; i < processes_count; i++)
    {
        int Childprocess = fork();

        if (Childprocess < 0) {
            // fork failed
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (Childprocess == 0){
            // child new process

            printf("child %d with PID (pid:%d)\n", i, (int) getpid());

            //running appropriate worker function as per user needed
            if (strcmp(task, "cpu") == 0){
                cpu(roll_cycles);
            }
            else if (strcmp(task, "mem") == 0){
                mem(roll_cycles);
            }
            else if (strcmp(task, "io") == 0)
            {
                io(roll_cycles);
            }


            exit(0);
        }
    }

    //waiting to finish all child processes
    for (int i = 0; i < processes_count; i++) {
        wait(NULL);
    }

    printf("\n  %d processes completed working succesfully.\n", processes_count);
}

