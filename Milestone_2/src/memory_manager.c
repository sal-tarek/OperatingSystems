#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"
#include "process.h"
#include "PCB.h"
#include "index.h"
#include "Queue.h"
#include "memory.h"


#define numProcesses 3
#define numQueues 4

// Global variables declared in main.c
extern Queue *job_pool;
extern MemoryWord *memory;
extern IndexEntry *index_table;
extern Queue *readyQueues[numQueues]; 
extern int clockCycle;

// Global array to store memory ranges for each process
MemoryRange ranges[MAX_PROCESSES];
int ranges_count = 0; // Number of processes with assigned ranges
int current_memory_usage = 0; // Track total memory words used

void readInstructions(Process *process) {
    // Temporary array to store variable names
    char **variables = (char**)calloc(10, sizeof(char*)); // Initial capacity for variables
    int var_capacity = 10;
    int var_count = 0;

    FILE *file = fopen(process->file_path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", process->file_path);
        free(variables);
        return;
    }

    // Count instructions and variables in one pass
    int inst_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        inst_count++;

        // Check for variables in "assign" instructions
        if (strncmp(line, "assign ", 7) == 0) {
            char *var_name = strtok(line + 7, " ");
            if (var_name) {
                // Check if variable already exists
                int exists = 0;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(variables[i], var_name) == 0) {
                        exists = 1;
                        break;
                    }
                }
                if (!exists) {
                    // // Resize if necessary
                    // if (var_count >= var_capacity) {
                    //     var_capacity *= 2;
                    //     char **new_vars = (char**)realloc(variables, var_capacity * sizeof(char*));
                    //     if (!new_vars) {
                    //         fclose(file);
                    //         for (int i = 0; i < var_count; i++) free(variables[i]);
                    //         free(variables);
                    //         return;
                    //     }
                    //     variables = new_vars;
                    // }
                    variables[var_count] = strdup(var_name);
                    var_count++;
                }
            }
        }
    }
    rewind(file); // Reset file pointer to read again for storage

    // Check if the process fits in memory
    int total_words_needed = inst_count + var_count + 1; // Instructions + Variables + PCB
    if (current_memory_usage + total_words_needed > MAX_MEMORY_WORDS) {
        fprintf(stderr, "Sorry, we can't store PID %d: we only have %d words left and the program needs %d words from memory\n",
                process->pid, MAX_MEMORY_WORDS - current_memory_usage, total_words_needed);
        fclose(file);
        free(variables);
        return;
    }

    // Assign starting address for instructions
    ranges[ranges_count].inst_start = (ranges_count == 0) ? 0 : ranges[ranges_count - 1].pcb_start + 1;
    ranges[ranges_count].inst_count = inst_count;

    // Read instructions and store them
    int inst_idx = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        addMemoryData(&memory, ranges[ranges_count].inst_start + inst_idx, line, TYPE_STRING);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", process->pid, inst_idx + 1);
        addIndexEntry(&index_table, key, ranges[ranges_count].inst_start + inst_idx);
        inst_idx++;
    }
    fclose(file);

    // Update burstTime with the number of instructions
    process->burstTime = ranges[ranges_count].inst_count;
    process->remainingTime = ranges[ranges_count].inst_count;

    // Assign addresses for variables
    ranges[ranges_count].var_start = ranges[ranges_count].inst_start + ranges[ranges_count].inst_count;
    ranges[ranges_count].var_count = var_count;
    for (int i = 0; i < var_count; i++) {
        addMemoryData(&memory, ranges[ranges_count].var_start + i, variables[i], TYPE_STRING);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Variable_%s", process->pid, variables[i]);
        addIndexEntry(&index_table, key, ranges[ranges_count].var_start + i);
        free(variables[i]);
    }
    free(variables);

    // Update memory usage
    current_memory_usage += total_words_needed;
}

void populatePCB(Process *process) {
    // Assign address for PCB
    ranges[ranges_count].pcb_start = ranges[ranges_count].var_start + ranges[ranges_count].var_count;
    ranges[ranges_count].pcb_count = 1;

    struct PCB *pcb = createPCBWithBounds(process->pid, ranges[ranges_count].inst_start, ranges[ranges_count].pcb_start + ranges[ranges_count].pcb_count - 1);
    if (pcb == NULL) {
        fprintf(stderr, "Failed to create PCB for PID: %d\n", process->pid);
        return;
    }

    setPCBState(pcb, READY);
    addMemoryData(&memory, ranges[ranges_count].pcb_start, pcb, TYPE_PCB);
    char key[32];
    snprintf(key, sizeof(key), "P%d_PCB", process->pid);
    addIndexEntry(&index_table, key, ranges[ranges_count].pcb_start);
}

void populateMemory() {
    if(!isEmpty(job_pool)){
        Process *curr;
        DataType type;
        int size = getQueueSize(job_pool);
        for (int i = 0; i < size && !isEmpty(job_pool); i++) {
            curr = peek(job_pool);
            if (curr->arrival_time <= clockCycle) {
                int pid = curr->pid - 1;
                if (pid < 0 || pid > 2) {
                    fprintf(stderr, "Invalid pid: %d\n", curr->pid);
                    dequeue(job_pool);
                    curr = peek(job_pool);
                    continue;
                }

                readInstructions(curr);
                populatePCB(curr);
                ranges_count++; // Increment ranges_count

                //dequeue from job pool
                dequeue(job_pool);
                curr->ready_time = clockCycle; // Set ready_time
                enqueue(readyQueues[0], curr); // Add to ready_queue
            }
            else{
                enqueue(job_pool, dequeue(job_pool)); // Re-enqueue the process
            }
        }
    }
    else{
        printf("Job pool is empty\n");
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
    for (int i = 0; i < ranges_count; i++) {
        int index = pid - 1;
        if (index == i) { // Match based on order of addition
            return ranges[i];
        }
    }
    MemoryRange invalid = {0, 0, 0, 0, 0, 0};
    return invalid;
}

void displayMemoryRange(int pid) {
    if (pid == 0) {
        for (int i = 0; i < ranges_count; i++) {
            // Since we process in order of arrival, use the index + 1 as the display PID
            int display_pid = i + 1;
            MemoryRange range = ranges[i];
            printf("P%d Memory Range:\n", display_pid);
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
// Function to free the memory ranges
void freeMemoryRanges() {
    for (int i = 0; i < ranges_count; i++) {
        // Free the PCB data
        MemoryWord *pcb_word = NULL;
        HASH_FIND_INT(memory, &ranges[i].pcb_start, pcb_word);
        if (pcb_word && pcb_word->data) {
            freePCB((PCB*)pcb_word->data);
        }
        // Free the instruction and variable data
        for (int j = 0; j < ranges[i].inst_count + ranges[i].var_count; j++) {
            MemoryWord *word = NULL;
            HASH_FIND_INT(memory, &j, word);
            if (word && word->data) {
                free(word->data);
            }
        }
    }
}