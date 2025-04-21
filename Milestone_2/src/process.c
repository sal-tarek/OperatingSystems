// process.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
//#include "PCB.h"


Process* createProcess(int pid, const char *file_path, int arrival_time, int burst_time) {
    Process* newProcess = (Process*)malloc(sizeof(Process));
    if (!newProcess) {
        fprintf(stderr, "Memory allocation for Process failed\n");
        exit(EXIT_FAILURE);
    }
    newProcess->pid = pid;
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "../programs/Program_%d.txt", pid);
    newProcess->file_path = strdup(full_path);
    newProcess->arrival_time = arrival_time;
    newProcess->ready_time = 0;
    newProcess->burstTime = burst_time;
    newProcess->remainingTime = burst_time;
    newProcess->state = NEW;
    newProcess->next = NULL;
    //newProcess->pcb = NULL;  // Initialize PCB pointer to NULL

    if (!newProcess->file_path) {
        fprintf(stderr, "Failed to allocate memory for file_path\n");
        free(newProcess);
        exit(EXIT_FAILURE);
    }

    return newProcess;
}

void displayProcess(Process *p) {
    if (p != NULL) {
        printf("Process ID: %d\n", p->pid);
        printf("State: %d\n", p->state);  // You can map this to names if needed (NEW, READY, etc.)
        printf("File Path: %s\n", p->file_path);
        printf("Arrival Time: %d\n", p->arrival_time);
        printf("Burst Time: %d\n", p->burstTime);
        printf("Remaining Time: %d\n", p->remainingTime);
        printf("------------------------\n");
    }
}