// memory_manager.h
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

void readInstructions(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range);
void populateVariables(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range);
void populatePCB(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range);
void populateMemory(Queue *job_pool, MemoryWord **memory, IndexEntry **index, int current_time);


#endif // MEMORY_MANAGER_H