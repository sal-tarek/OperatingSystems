// process.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "memory_manager.h"


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

        // Prepare the key string for PCB (e.g., "P3_PCB")
        char key[20];
        snprintf(key, sizeof(key), "P%d_PCB", p->pid);

        // Specify the type you're looking for
        DataType type = TYPE_PCB;

        // Fetch the PCB using the formatted key
        struct PCB *pcb = (struct PCB *)fetchDataByIndex(key, &type);

        // Make sure pcb is not NULL before accessing it
        if (pcb != NULL) {
            printf("PCB State: %d\n", pcb->state);
        } else {
            printf("PCB not found for PID %d\n", p->pid);
        }

        printf("File Path: %s\n", p->file_path);
        printf("Arrival Time: %d\n", p->arrival_time);
        printf("Burst Time: %d\n", p->burstTime);
        printf("Remaining Time: %d\n", p->remainingTime);
        printf("------------------------\n");
    }
}

void freeProcess(Process *p) {
    if (p != NULL) {
        free(p->file_path);
        free(p);
    }
}