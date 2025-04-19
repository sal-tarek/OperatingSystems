// memory_manager.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"
#include "process.h"
#include "PCB.h"
#include "Queue.h"
#include "memory.h"
#include "index.h"
 

// Hardcoded ranges for P1, P2, P3
static MemoryRange ranges[] = {
    {0, 7, 7, 2, 9, 6},    // P1: inst 0–6, var 7–8 (a, b), PCB 9–14
    {15, 7, 22, 2, 24, 6}, // P2: inst 15–21, var 22–23 (a, b), PCB 24–29
    {30, 9, 39, 2, 41, 6}  // P3: inst 30–38, var 39–40 (a, b), PCB 41–46
};

void readInstructions(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range) {
    char **variables = (char**)calloc(range.var_count, sizeof(char*));
    int var_count = 0;

    FILE *file = fopen(process->file_path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", process->file_path);
        free(variables);
        return;
    }

    char line[256];
    int inst_idx = 0;
    while (fgets(line, sizeof(line), file) && inst_idx < range.inst_count) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        addMemoryData(memory, range.inst_start + inst_idx, line);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", process->pid, inst_idx + 1);
        addIndexEntry(index, key, range.inst_start + inst_idx);

        // Extract variables from 'assign' instructions
        if (strncmp(line, "assign ", 7) == 0 && var_count < range.var_count) {
            char *var_name = strtok(line + 7, " ");
            if (var_name) {
                variables[var_count] = strdup(var_name);
                var_count++;
            }
        }
        inst_idx++;
    }
    fclose(file);

    // Store variables in memory and index
    for (int i = 0; i < var_count; i++) {
        addMemoryData(memory, range.var_start + i, variables[i]);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Variable_%d", process->pid, i + 1);
        addIndexEntry(index, key, range.var_start + i);
        free(variables[i]);
    }
    free(variables);
}

void populateVariables(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range) {
    // No-op: Variables are now handled in readInstructions
}

void populatePCB(Process *process, MemoryWord **memory, IndexEntry **index, MemoryRange range) {
    PCB *pcb = createPCB(process->pid);
    char data[32];
    char key[32];

    snprintf(data, sizeof(data), "PID:%d", pcb->id);
    addMemoryData(memory, range.pcb_start + 0, data);
    snprintf(key, sizeof(key), "P%d_PCB_1", process->pid);
    addIndexEntry(index, key, range.pcb_start + 0);

    snprintf(data, sizeof(data), "State:%s", pcb->state == NEW ? "NEW" : "READY");
    addMemoryData(memory, range.pcb_start + 1, data);
    snprintf(key, sizeof(key), "P%d_PCB_2", process->pid);
    addIndexEntry(index, key, range.pcb_start + 1);

    snprintf(data, sizeof(data), "Priority:%d", pcb->priority);
    addMemoryData(memory, range.pcb_start + 2, data);
    snprintf(key, sizeof(key), "P%d_PCB_3", process->pid);
    addIndexEntry(index, key, range.pcb_start + 2);

    snprintf(data, sizeof(data), "PC:%d", pcb->programCounter);
    addMemoryData(memory, range.pcb_start + 3, data);
    snprintf(key, sizeof(key), "P%d_PCB_4", process->pid);
    addIndexEntry(index, key, range.pcb_start + 3);

    snprintf(data, sizeof(data), "MemLower:%d", range.inst_start);
    addMemoryData(memory, range.pcb_start + 4, data);
    snprintf(key, sizeof(key), "P%d_PCB_5", process->pid);
    addIndexEntry(index, key, range.pcb_start + 4);

    snprintf(data, sizeof(data), "MemUpper:%d", range.pcb_start + range.pcb_count - 1);
    addMemoryData(memory, range.pcb_start + 5, data);
    snprintf(key, sizeof(key), "P%d_PCB_6", process->pid);
    addIndexEntry(index, key, range.pcb_start + 5);

    free(pcb);
}

void populateMemory(Queue *job_pool, MemoryWord **memory, IndexEntry **index, int current_time) {
    Process *curr = job_pool->front;

    while (curr != NULL) {
        if (curr->state == NEW && curr->arrival_time <= current_time) {
            int pid = curr->pid - 1;
            if (pid < 0 || pid > 2) {
                fprintf(stderr, "Invalid pid: %d\n", curr->pid);
                curr = curr->next;
                continue;
            }

            MemoryRange range = ranges[pid];
            readInstructions(curr, memory, index, range);
            populatePCB(curr, memory, index, range);

            curr->state = READY;
            //remove from job pool
            dequeue(job_pool);
            //add to ready queue
        }
        curr = curr->next;
    }
}