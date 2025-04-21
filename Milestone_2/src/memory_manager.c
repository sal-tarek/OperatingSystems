#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"
#include "process.h"
#include "PCB.h"

#define numProcesses 3
#define numQueues 4

// Global variables declared in main.c
extern Queue *job_pool;
extern MemoryWord *memory;
extern IndexEntry *index_table;
extern Queue *readyQueues[numQueues]; 
extern int clockCycle;

// Hardcoded ranges for P1, P2, P3
static MemoryRange ranges[] = {
    {0, 7, 7, 2, 9, 6},    // P1: inst 0–6, var 7–8 (a, b), PCB 9
    {15, 7, 22, 2, 24, 6}, // P2: inst 15–21, var 22–23 (a, b), PCB 24
    {30, 9, 39, 2, 41, 6}  // P3: inst 30–38, var 39–40 (a, b), PCB 41
};

void readInstructions(Process *process, MemoryRange range) {
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
        line[strcspn(line, "\n")] = 0;
        addMemoryData(&memory, range.inst_start + inst_idx, line, TYPE_STRING);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", process->pid, inst_idx + 1);
        addIndexEntry(&index_table, key, range.inst_start + inst_idx);

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

    for (int i = 0; i < var_count; i++) {
        addMemoryData(&memory, range.var_start + i, variables[i], TYPE_STRING);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Variable_%d", process->pid, i + 1);
        addIndexEntry(&index_table, key, range.var_start + i);
        free(variables[i]);
    }
    free(variables);
}

void populateVariables(Process *process, MemoryRange range) {
    // No-op: Variables handled in readInstructions
}

void populatePCB(Process *process, MemoryRange range) {
    struct PCB *pcb = createPCBWithBounds(process->pid, range.inst_start, range.pcb_start + range.pcb_count - 1);
    if (pcb == NULL) {
        fprintf(stderr, "Failed to create PCB for PID: %d\n", process->pid);
        return;
    }

    setPCBState(pcb, READY); // Set PCB state to READY before storing in memory

    addMemoryData(&memory, range.pcb_start, pcb, TYPE_PCB);
    char key[32];
    snprintf(key, sizeof(key), "P%d_PCB", process->pid);
    addIndexEntry(&index_table, key, range.pcb_start);
}

void populateMemory() {
    Process *curr = peek(job_pool);
    DataType type;
    for (int i = 0; i < 3 ; i++) {
        if (curr->arrival_time == clockCycle) {
            int pid = curr->pid - 1;
            if (pid < 0 || pid > 2) {
                fprintf(stderr, "Invalid pid: %d\n", curr->pid);
                dequeue(job_pool);
                curr = peek(job_pool);
                continue;
            }

            MemoryRange range = ranges[pid];
            readInstructions(curr, range);
            populatePCB(curr, range);
            //dequeue from job pool
            dequeue(job_pool);
            curr->ready_time = clockCycle; // Set ready_time
            enqueue(readyQueues[0], curr); // Add to ready_queue
        }
        enqueue(job_pool, dequeue(job_pool)); // Re-enqueue the process
        curr = peek(job_pool);
      }
    }
   
void* fetchDataByIndex(const char *key, DataType *type_out) {
    int address = getIndexAddress(index_table, key);
    if (address == -1) {
        fprintf(stderr, "Key not found, it is not yet stored in memory: %s\n", key);
        return NULL;
    }

    MemoryWord *word = NULL;
    HASH_FIND_INT(memory, &address, word);
    if (!word || !word->data) {
        fprintf(stderr, "No data at address: %d\n", address);
        return NULL;
    }

    if (type_out) {
        *type_out = word->type;
    }
    return word->data;
}

int updateDataByIndex(const char *key, void *new_data, DataType type) {
    if (strstr(key, "_Instruction_") != NULL) {
        fprintf(stderr, "Cannot update instruction key: %s\n", key);
        return -1;
    }

    int address = getIndexAddress(index_table, key);
    if (address == -1) {
        fprintf(stderr, "Key not found: %s\n", key);
        return -1;
    }

    // Validate key and type consistency
    if (strstr(key, "_PCB") != NULL) {
        if (type != TYPE_PCB) {
            fprintf(stderr, "Invalid type for PCB key: %s, expected TYPE_PCB\n", key);
            return -1;
        }
        if (new_data == NULL) {
            fprintf(stderr, "NULL PCB data for key: %s\n", key);
            return -1;
        }
    } else if (strstr(key, "_Variable_") != NULL) {
        if (type != TYPE_STRING) {
            fprintf(stderr, "Invalid type for variable key: %s, expected TYPE_STRING\n", key);
            return -1;
        }
        if (new_data == NULL) {
            fprintf(stderr, "NULL string data for key: %s\n", key);
            return -1;
        }
    } else {
        fprintf(stderr, "Invalid key: %s, must be _PCB or _Variable_\n", key);
        return -1;
    }

    if (updateMemoryData(&memory, address, new_data, type) != 0) {
        fprintf(stderr, "Failed to update data at address: %d for key: %s\n", address, key);
        return -1;
    }

    return 0;
}

MemoryRange getProcessMemoryRange(int pid) {
    int index = pid - 1;
    if (index < 0 || index > 2) {
        fprintf(stderr, "Invalid PID: %d\n", pid);
        MemoryRange invalid = {0, 0, 0, 0, 0, 0};
        return invalid;
    }
    return ranges[index];
}

void displayMemoryRange(int pid) {
    if (pid == 0) {
        for (int i = 0; i < 3; i++) {
            MemoryRange range = ranges[i];
            printf("P%d Memory Range:\n", i + 1);
            printf("  Instructions: %d–%d (Count: %d)\n", 
                   range.inst_start, range.inst_start + range.inst_count - 1, range.inst_count);
            printf("  Variables: %d–%d (Count: %d)\n", 
                   range.var_start, range.var_start + range.var_count - 1, range.var_count);
            printf("  PCB: %d (Count: %d)\n", 
                   range.pcb_start, range.pcb_count);
        }
    } else {
        MemoryRange range = getProcessMemoryRange(pid);
        if (range.inst_count == 0 && range.var_count == 0 && range.pcb_count == 0) {
            fprintf(stderr, "No memory range found for PID: %d\n", pid);
            return;
        }
        printf("P%d Memory Range:\n", pid);
        printf("  Instructions: %d–%d (Count: %d)\n", 
               range.inst_start, range.inst_start + range.inst_count - 1, range.inst_count);
        printf("  Variables: %d–%d (Count: %d)\n", 
               range.var_start, range.var_start + range.var_count - 1, range.var_count);
        printf("  PCB: %d (Count: %d)\n", 
               range.pcb_start, range.pcb_count);
    }
}