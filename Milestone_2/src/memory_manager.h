#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "process.h"
#include "memory.h"

typedef struct {
    int inst_start;  // Start address for instructions
    int inst_count;  // Number of instructions
    int var_start;   // Start address for variables
    int var_count;   // Number of variables
    int pcb_start;   // Start address for PCB
    int pcb_count;   // Number of PCB entries (typically 1)
} MemoryRange;

#define MAX_PROCESSES 10 // Maximum number of processes to support
#define MAX_MEMORY_WORDS 60 // Maximum memory size

extern MemoryRange ranges[MAX_PROCESSES]; // Store ranges for each process
extern int ranges_count; // Number of processes with assigned ranges
extern int current_memory_usage; // Track total memory words used

void readInstructions(Process *process);
void populatePCB(Process *process);
void populateMemory(void);
void* fetchDataByIndex(const char *key, DataType *type_out);
int updateDataByIndex(const char *key, void *new_data, DataType type);
MemoryRange getProcessMemoryRange(int pid);
void displayMemoryRange(int pid);
void freeMemoryRanges();

#endif