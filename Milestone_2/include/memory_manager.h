#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "memory.h"
#include "Queue.h"
#include "PCB.h"
#include "process.h"
#include "index.h"

// Constants
#define MAX_MEMORY_WORDS 60
#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support

// Structure for memory ranges
typedef struct {
    int pid; // Add PID to track which process this range belongs to
    int inst_start;
    int inst_count;
    int var_start;
    int var_count;
    int pcb_start;
    int pcb_count;
} MemoryRange;

// Global variables (declared in main.c)
extern Queue *job_pool;
extern Queue *readyQueues[4];
extern MemoryWord *memory;
extern IndexEntry *index_table;
extern int clockCycle; 
extern MemoryRange ranges[MAX_NUM_PROCESSES];
extern int ranges_count;
extern int current_memory_usage;

// Function declarations
void readInstructionsOnly(Process *process);
void addInstVarsPCB(Process *process);
void populateMemory();
void* fetchDataByIndex(const char *key, DataType *type_out);
int updateDataByIndex(const char *key, void *new_data, DataType type);
MemoryRange getProcessMemoryRange(int pid);
void displayMemoryRange(int pid);
void freeMemoryRanges();
void deleteProcessFromMemory(int pid);
void resetMemory();

#endif