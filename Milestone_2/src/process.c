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

    // Set file path using the parameter
    if (file_path != NULL) {
        newProcess->file_path = strdup(file_path);
        if (!newProcess->file_path) {
            fprintf(stderr, "Failed to allocate memory for file_path\n");
            free(newProcess);
            exit(EXIT_FAILURE);
        }
    } else {
        // Handle NULL file_path by providing a default path or error
        char full_path[64];
        snprintf(full_path, sizeof(full_path), "../programs/Program_%d.txt", pid);
        newProcess->file_path = strdup(full_path);
        if (!newProcess->file_path) {
            fprintf(stderr, "Failed to allocate memory for file_path\n");
            free(newProcess);
            exit(EXIT_FAILURE);
        }
    }

    newProcess->state = NEW;
    newProcess->arrival_time = arrival_time;
    newProcess->ready_time = 0;
    newProcess->burstTime = 0;
    newProcess->remainingTime = 0;
    newProcess->next = NULL;
    newProcess->quantumUsed = 0;
    newProcess->timeInQueue = 0;
    newProcess->instruction_count = 0;
    newProcess->instructions = NULL; // Initialize to NULL
    newProcess->variable_count = 0;
    newProcess->variables = NULL; // Initialize to NULL

    // Read instructions and variables from the file
    readInstructionsOnly(newProcess);

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
            case BLOCKED: printf("State: WAITING\n"); break;
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
        printf("instructions: %s\n", p->instructions);
        printf("------------------------\n");
    }
}

void freeProcess(Process *p) {
    if (p == NULL) return;

    free(p->file_path);
    free(p->instructions);
    if (p->variables) {
        for (int i = 0; i < p->variable_count; i++) {
            free(p->variables[i]);
        }
        free(p->variables);
    }
    free(p);
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

Process *cloneProcess(Process *original) {
    if (original == NULL) {
        return NULL;
    }

    // Allocate new Process struct
    Process *clone = (Process *)malloc(sizeof(Process));
    if (clone == NULL) {
        fprintf(stderr, "Failed to allocate memory for cloned process\n");
        return NULL;
    }

    // Copy scalar values
    clone->pid = original->pid;
    clone->state = original->state;
    clone->arrival_time = original->arrival_time;
    clone->ready_time = original->ready_time;
    clone->burstTime = original->burstTime;
    clone->remainingTime = original->remainingTime;
    clone->quantumUsed = original->quantumUsed;
    clone->timeInQueue = original->timeInQueue;
    clone->instruction_count = original->instruction_count;
    clone->variable_count = original->variable_count;

    // Deep copy file_path
    clone->file_path = original->file_path ? strdup(original->file_path) : NULL;
    if (original->file_path && clone->file_path == NULL) {
        fprintf(stderr, "Failed to copy file_path for PID %d\n", clone->pid);
        free(clone);
        return NULL;
    }

    // Deep copy instructions
    clone->instructions = original->instructions ? strdup(original->instructions) : NULL;
    if (original->instructions && clone->instructions == NULL) {
        fprintf(stderr, "Failed to copy instructions for PID %d\n", clone->pid);
        free(clone->file_path);
        free(clone);
        return NULL;
    }

    // Deep copy variables array
    if (original->variable_count > 0 && original->variables) {
        clone->variables = (char **)calloc(original->variable_count, sizeof(char *));
        if (clone->variables == NULL) {
            fprintf(stderr, "Failed to allocate variables array for PID %d\n", clone->pid);
            free(clone->file_path);
            free(clone->instructions);
            free(clone);
            return NULL;
        }
        for (int i = 0; i < original->variable_count; i++) {
            clone->variables[i] = original->variables[i] ? strdup(original->variables[i]) : NULL;
            if (original->variables[i] && clone->variables[i] == NULL) {
                fprintf(stderr, "Failed to copy variable %d for PID %d\n", i, clone->pid);
                for (int j = 0; j < i; j++) {
                    free(clone->variables[j]);
                }
                free(clone->variables);
                free(clone->file_path);
                free(clone->instructions);
                free(clone);
                return NULL;
            }
        }
    } else {
        clone->variables = NULL;
    }

    // Initialize next pointer
    clone->next = NULL;

    return clone;
}