#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "process.h"
#include "PCB.h"
#include "memory.h"
#include "index.h"
#include "Queue.h"

typedef struct {
    int inst_start;
    int inst_count;
    int var_start;
    int var_count;
    int pcb_start;
    int pcb_count;
} MemoryRange;

void readInstructions(Process *process, MemoryRange range);
void populateVariables(Process *process, MemoryRange range);
void populatePCB(Process *process, MemoryRange range);
void populateMemory(int current_time);
void* fetchDataByIndex(const char *key, DataType *type_out);
int updateDataByIndex(const char *key, void *new_data, DataType type);
MemoryRange getProcessMemoryRange(int pid);
void displayMemoryRange(int pid);

#endif // MEMORY_MANAGER_H