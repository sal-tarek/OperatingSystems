#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

Process* createProcess(int pid, const char *file_path, int arrival_time, int burst_time) {
    Process* newProcess = (Process*)malloc(sizeof(Process));
    if (!newProcess) {
        fprintf(stderr, "Memory allocation for Process failed\n");
        exit(EXIT_FAILURE);
    }

    newProcess->pid = pid;
    newProcess->file_path = strdup(file_path);
    newProcess->arrival_time = arrival_time;
    newProcess->ready_time = 0;
    newProcess->burstTime = burst_time;
    newProcess->remainingTime = burst_time;
    newProcess->state = NEW;
    newProcess->next = NULL;

    if (!newProcess->pid || !newProcess->file_path) {
        fprintf(stderr, "Failed to allocate memory for pid or file_path\n");
        free(newProcess->pid);
        free(newProcess->file_path);
        free(newProcess);
        exit(EXIT_FAILURE);
    }

    return newProcess;
}
