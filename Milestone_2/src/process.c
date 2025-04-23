#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "memory_manager.h"
#include "memory.h"
#include "index.h"

Process* createProcess(int pid, const char *file_path, int arrival_time) {
    Process* newProcess = (Process*)malloc(sizeof(Process));
    if (!newProcess) {
        fprintf(stderr, "Memory allocation for Process failed\n");
        exit(EXIT_FAILURE);
    }
    newProcess->pid = pid;
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "../programs/Program_%d.txt", pid);
    newProcess->file_path = strdup(full_path);
    newProcess->state = NEW;
    newProcess->arrival_time = arrival_time;
    newProcess->ready_time = 0;
    newProcess->burstTime = 0;
    newProcess->remainingTime = 0;
    newProcess->next = NULL;
    newProcess->quantumUsed = 0;

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
        char key[20];
        ProcessState state = p->state;
        switch (state) {
            case NEW: printf("State: NEW\n"); break;
            case READY: printf("State: READY\n"); break;
            case RUNNING: printf("State: RUNNING\n"); break;
            case WAITING: printf("State: WAITING\n"); break;
            case TERMINATED: printf("State: TERMINATED\n"); break;
            default: printf("State: UNKNOWN\n");
        }
        DataType type;
        struct PCB *pcb = (struct PCB *)fetchDataByIndex(key, &type);
        if (pcb != NULL && type == TYPE_PCB) {
            printf("PCB State: %d\n", getPCBState(pcb));
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

void setProcessState(int pid, ProcessState newState) {
    char key[20];
    snprintf(key, sizeof(key), "P%d_PCB", pid);
    DataType type;
    void *data = fetchDataByIndex(key, &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        setPCBState(pcb, newState);
    } else {
        fprintf(stderr, "Failed to update PCB state for PID %d\n", pid);
    }
}

ProcessState getProcessState(int pid) {
    char key[20];
    snprintf(key, sizeof(key), "P%d_PCB", pid);
    DataType type;
    void *data = fetchDataByIndex(key, &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        return getPCBState(pcb);
    } else {
        fprintf(stderr, "Failed to fetch PCB for PID %d\n", pid);
        return ERROR;
    }
}

// Fetch priority from pcb
int getProcessPriority(int pid) {
    char key[20];
    snprintf(key, sizeof(key), "P%d_PCB", pid);
    DataType type;
    void *data = fetchDataByIndex(key, &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        return getPCBPriority(pcb);
    } else {
        fprintf(stderr, "Failed to fetch PCB for PID %d\n", pid);
        return ERROR;
    }
}

// Set priority in pcb
void setProcessPriority(int pid, int newPriority) {
    char key[20];
    snprintf(key, sizeof(key), "P%d_PCB", pid);
    DataType type;
    void *data = fetchDataByIndex(key, &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        setPCBPriority(pcb, newPriority);
    } else {
        fprintf(stderr, "Failed to update PCB priority for PID %d\n", pid);
    }
}