#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"
#include "process.h"
#include "PCB.h"
#include "index.h"
#include "Queue.h"
#include "memory.h"

#define MAX_NUM_PROCESSES 10 // Maximum number of processes to support
#define MAX_NUM_QUEUES 4     // Maximum number of queues
#define QUEUE_CAPACITY 10

// Global variables declared in main.c
extern Queue *job_pool;
extern MemoryWord *memory;
extern IndexEntry *index_table;
extern Queue *readyQueues[MAX_NUM_QUEUES];
extern int clockCycle;
extern Process *processes[MAX_NUM_PROCESSES];
extern int numberOfProcesses;

// Global array to store memory ranges for each process
MemoryRange ranges[MAX_NUM_PROCESSES];
int ranges_count = 0;         // Number of processes with assigned ranges
int current_memory_usage = 0; // Track total memory words used

// Forward declarations
void resetMemoryRanges(void);
void resetProcessList(void);

void readInstructionsOnly(Process *process)
{
    // Temporary array to store variable names
    char **variables = (char **)calloc(10, sizeof(char *)); // Initial capacity for variables
    int var_capacity = 10;
    int var_count = 0;

    FILE *file = fopen(process->file_path, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open %s\n", process->file_path);
        free(variables);
        return;
    }

    // Count instructions and variables in one pass
    int inst_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;
        inst_count++;

        // add the line to instructions string in process struct
        if (process->instructions == NULL)
        {
            // First line, allocate memory
            process->instructions = strdup(line);
        }
        else
        {
            // Reallocate memory to fit existing content plus new line
            size_t new_size = strlen(process->instructions) + strlen(line) + 2; // +2 for newline and null terminator
            char *temp = realloc(process->instructions, new_size);

            if (temp != NULL)
            {
                process->instructions = temp;
                strcat(process->instructions, "\n"); // Add newline separator
                strcat(process->instructions, line); // Append the new line
            }
            else
            {
                // Handle memory allocation failure
                // (could add error handling here)
            }
        }

        // Check for variables in "assign" instructions
        if (strncmp(line, "assign ", 7) == 0)
        {
            char *var_name = strtok(line + 7, " ");
            if (var_name)
            {
                // Check if variable already exists
                int exists = 0;
                for (int i = 0; i < var_count; i++)
                {
                    if (strcmp(variables[i], var_name) == 0)
                    {
                        exists = 1;
                        break;
                    }
                }
                if (!exists)
                {
                    variables[var_count] = strdup(var_name);
                    var_count++;
                }
            }
        }
    }
    fclose(file);
    // assign instructions and variables counts to the process
    process->instruction_count = inst_count;
    process->variable_count = var_count;
    process->variables = (char **)calloc(var_count, sizeof(char *));
    for (int i = 0; i < var_count; i++)
    {
        process->variables[i] = strdup(variables[i]);
    }
}

void addInstVarsPCB(Process *process)
{
    // Check if the process fits in memory
    int inst_count = process->instruction_count;
    int var_count = process->variable_count;
    char **variables = process->variables;
    if (variables == NULL)
    {
        fprintf(stderr, "No variables found for PID %d\n", process->pid);
        return;
    }
    // Check if the process can fit in memory
    int total_words_needed = inst_count + var_count + 1; // Instructions + Variables + PCB
    if (current_memory_usage + total_words_needed > MAX_MEMORY_WORDS)
    {
        fprintf(stderr, "Sorry, we can't store PID %d: we only have %d words left and the program needs %d words from memory\n",
                process->pid, MAX_MEMORY_WORDS - current_memory_usage, total_words_needed);
        process->variables = NULL;      // Set to NULL to indicate no variables
        process->instruction_count = 0; // Reset instruction count
        process->variable_count = 0;    // Reset variable count
        return;
    }

    // Step 1: Allocate PCB ranges
    ranges[ranges_count].pid = process->pid; // Store the PID
    ranges[ranges_count].pcb_start = (ranges_count == 0) ? 0 : ranges[ranges_count - 1].var_start + ranges[ranges_count - 1].var_count;
    ranges[ranges_count].pcb_count = 1;

    // Step 2: Allocate instructions ranges after PCB
    ranges[ranges_count].inst_start = ranges[ranges_count].pcb_start + ranges[ranges_count].pcb_count;
    ranges[ranges_count].inst_count = inst_count;

    char *instruction_ptr = process->instructions;
    char line[256];
    int inst_idx = 0;

    // Update burstTime with the number of instructions
    process->burstTime = ranges[ranges_count].inst_count;
    process->remainingTime = ranges[ranges_count].inst_count;

    // Step 3: Allocate variables ranges after instructions
    ranges[ranges_count].var_start = ranges[ranges_count].inst_start + ranges[ranges_count].inst_count;
    ranges[ranges_count].var_count = var_count;

    // Create and store the PCB
    int lower_bound = ranges[ranges_count].pcb_start;                                      // Start of PCB
    int upper_bound = ranges[ranges_count].var_start + ranges[ranges_count].var_count - 1; // End of variables
    struct PCB *pcb = createPCBWithBounds(process->pid, lower_bound, upper_bound);
    setPCBState(pcb, READY);
    process->state = READY;
    addMemoryData(&memory, ranges[ranges_count].pcb_start, pcb, TYPE_PCB);
    char pcb_key[32];
    snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", process->pid);
    addIndexEntry(&index_table, pcb_key, ranges[ranges_count].pcb_start);

    // If we have stored instructions
    if (instruction_ptr)
    {
        while (*instruction_ptr)
        {
            // Clear the line buffer
            memset(line, 0, sizeof(line));
            // Copy characters until we hit a newline or end of string
            int line_idx = 0;
            while (*instruction_ptr && *instruction_ptr != '\n' && line_idx < sizeof(line) - 1)
            {
                line[line_idx++] = *instruction_ptr++;
            }
            // Add null terminator
            line[line_idx] = 0;
            // Skip the newline character if present
            if (*instruction_ptr == '\n')
            {
                instruction_ptr++;
            }
            // Add the line to memory
            addMemoryData(&memory, ranges[ranges_count].inst_start + inst_idx, line, TYPE_STRING);
            // Create and add index entry
            char key[32];
            snprintf(key, sizeof(key), "P%d_Instruction_%d", process->pid, inst_idx + 1);
            addIndexEntry(&index_table, key, ranges[ranges_count].inst_start + inst_idx);
            inst_idx++;
        }
    }

    // Store variables in memory
    for (int i = 0; i < var_count; i++)
    {
        addMemoryData(&memory, ranges[ranges_count].var_start + i, "Variable Not Initialized", TYPE_STRING);
        char key[32];
        snprintf(key, sizeof(key), "P%d_Variable_%s", process->pid, variables[i]);
        addIndexEntry(&index_table, key, ranges[ranges_count].var_start + i);
        free(variables[i]);
    }
    free(variables);

    // Update memory usage
    current_memory_usage += total_words_needed;
}

void populateMemory()
{
    DataType type;
    int size = getQueueSize(job_pool);
    for (int i = 0; i < size; i++)
    {
        Process *curr = peek(job_pool);

        if (curr->arrival_time <= clockCycle)
        {
            addInstVarsPCB(curr); // This now handles PCB, instructions, and variables
            ranges_count++;       // Increment ranges_count

            // Dequeue from job pool
            dequeue(job_pool);

            // Set the process state to READY
            curr->state = READY;
            curr->ready_time = clockCycle; // Set ready_time

            enqueue(readyQueues[0], curr); // Add to ready_queue

            processes[numberOfProcesses] = curr; // Store the process in the global array
            numberOfProcesses++;
            printMemory();
        }
        else
        {
            enqueue(job_pool, dequeue(job_pool)); // Re-enqueue the process
        }
    }
}

void *fetchDataByIndex(const char *key, DataType *type_out)
{
    int address = getIndexAddress(index_table, key);
    if (address == -1)
    {
        fprintf(stderr, "Key not found, it is not yet stored in memory: %s\n", key);
        return NULL;
    }

    MemoryWord *word = NULL;
    HASH_FIND_INT(memory, &address, word);
    if (!word || !word->data)
    {
        fprintf(stderr, "No data at address: %d\n", address);
        return NULL;
    }

    if (type_out)
    {
        *type_out = word->type;
    }
    return word->data;
}

int updateDataByIndex(const char *key, void *new_data, DataType type)
{
    if (strstr(key, "_Instruction_") != NULL)
    {
        fprintf(stderr, "Cannot update instruction key: %s\n", key);
        return -1;
    }

    int address = getIndexAddress(index_table, key);
    if (address == -1)
    {
        fprintf(stderr, "Key not found: %s\n", key);
        return -1;
    }

    // Validate key and type consistency
    if (strstr(key, "_PCB") != NULL)
    {
        if (type != TYPE_PCB)
        {
            fprintf(stderr, "Invalid type for PCB key: %s, expected TYPE_PCB\n", key);
            return -1;
        }
        if (new_data == NULL)
        {
            fprintf(stderr, "NULL PCB data for key: %s\n", key);
            return -1;
        }
    }
    else if (strstr(key, "_Variable_") != NULL)
    {
        if (type != TYPE_STRING)
        {
            fprintf(stderr, "Invalid type for variable key: %s, expected TYPE_STRING\n", key);
            return -1;
        }
        if (new_data == NULL)
        {
            fprintf(stderr, "NULL string data for key: %s\n", key);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Invalid key: %s, must be _PCB or _Variable_\n", key);
        return -1;
    }

    if (updateMemoryData(&memory, address, new_data, type) != 0)
    {
        fprintf(stderr, "Failed to update data at address: %d for key: %s\n", address, key);
        return -1;
    }

    return 0;
}

MemoryRange getProcessMemoryRange(int pid)
{
    for (int i = 0; i < ranges_count; i++)
    {
        if (ranges[i].pid == pid)
        {
            return ranges[i];
        }
    }
    MemoryRange invalid = {0, 0, 0, 0, 0, 0, 0};
    return invalid;
}

void displayMemoryRange(int pid)
{
    if (pid == 0)
    {
        for (int i = 0; i < ranges_count; i++)
        {
            MemoryRange range = ranges[i];
            printf("P%d Memory Range:\n", range.pid);
            printf("  PCB: %d (Count: %d)\n",
                   range.pcb_start, range.pcb_count);
            printf("  Instructions: %d–%d (Count: %d)\n",
                   range.inst_start, range.inst_start + range.inst_count - 1, range.inst_count);
            printf("  Variables: %d–%d (Count: %d)\n",
                   range.var_start, range.var_start + range.var_count - 1, range.var_count);
        }
    }
    else
    {
        MemoryRange range = getProcessMemoryRange(pid);
        if (range.inst_count == 0 && range.var_count == 0 && range.pcb_count == 0)
        {
            fprintf(stderr, "No memory range found for PID: %d\n", pid);
            return;
        }
        printf("P%d Memory Range:\n", pid);
        printf("  PCB: %d (Count: %d)\n",
               range.pcb_start, range.pcb_count);
        printf("  Instructions: %d–%d (Count: %d)\n",
               range.inst_start, range.inst_start + range.inst_count - 1, range.inst_count);
        printf("  Variables: %d–%d (Count: %d)\n",
               range.var_start, range.var_start + range.var_count - 1, range.var_count);
    }
}

// Function to free the memory ranges
void freeMemoryRanges()
{
    for (int i = 0; i < ranges_count; i++)
    {
        // Free the PCB data
        MemoryWord *pcb_word = NULL;
        HASH_FIND_INT(memory, &ranges[i].pcb_start, pcb_word);
        if (pcb_word && pcb_word->data)
        {
            freePCB((PCB *)pcb_word->data);
        }

        // Free the instruction data
        for (int j = ranges[i].inst_start; j < ranges[i].inst_start + ranges[i].inst_count; j++)
        {
            MemoryWord *word = NULL;
            HASH_FIND_INT(memory, &j, word);
            if (word && word->data)
            {
                free(word->data);
                HASH_DEL(memory, word);
                free(word);
            }
        }

        // Free the variable data
        for (int j = ranges[i].var_start; j < ranges[i].var_start + ranges[i].var_count; j++)
        {
            MemoryWord *word = NULL;
            HASH_FIND_INT(memory, &j, word);
            if (word && word->data)
            {
                free(word->data);
                HASH_DEL(memory, word);
                free(word);
            }
        }
    }
}


void deleteProcessFromMemory(int pid)
{
    // Step 1: Find the process's memory range
    if (pid < 0 || pid >= ranges_count || ranges[pid].pid == -1)
    {
        fprintf(stderr, "No memory range found for PID: %d\n", pid);
        return;
    }
    MemoryRange range = ranges[pid];

    // Step 2: Free the memory range from the memory hash
    for (int i = range.pcb_start; i < range.pcb_start + range.pcb_count; i++)
    {
        MemoryWord *slot;
        HASH_FIND_INT(memory, &i, slot);
        if (slot)
        {
            if (slot->type == TYPE_PCB && slot->data)
            {
                free(slot->data); // Free the PCB data
            }
            HASH_DEL(memory, slot); // Remove the slot from the hash
            free(slot);             // Free the MemoryWord struct
        }
    }
    for (int i = range.inst_start; i < range.inst_start + range.inst_count; i++)
    {
        MemoryWord *slot;
        HASH_FIND_INT(memory, &i, slot);
        if (slot)
        {
            if (slot->type == TYPE_STRING && slot->data)
            {
                free(slot->data); // Free the instruction string
            }
            HASH_DEL(memory, slot); // Remove the slot from the hash
            free(slot);             // Free the MemoryWord struct
        }
    }
    for (int i = range.var_start; i < range.var_start + range.var_count; i++)
    {
        MemoryWord *slot;
        HASH_FIND_INT(memory, &i, slot);
        if (slot)
        {
            if (slot->type == TYPE_STRING && slot->data)
            {
                free(slot->data); // Free the variable string
            }
            HASH_DEL(memory, slot); // Remove the slot from the hash
            free(slot);             // Free the MemoryWord struct
        }
    }

    // Step 3: Remove index entries for this process
    IndexEntry *entry, *tmp;
    char prefix[32];
    snprintf(prefix, sizeof(prefix), "P%d_", pid);
    HASH_ITER(hh, index_table, entry, tmp)
    {
        if (strncmp(entry->key, prefix, strlen(prefix)) == 0)
        {
            HASH_DEL(index_table, entry); // Remove from index hash
            free(entry->key);             // Free the key string
            free(entry);                  // Free the entry
        }
    }

    // Step 4: Update memory usage
    int total_words_freed = range.pcb_count + range.inst_count + range.var_count;
    current_memory_usage -= total_words_freed;

    // Step 5: Clear the range entry
    ranges[pid].pid = -1; // Mark as unused
    ranges[pid].pcb_count = 0;
    ranges[pid].inst_count = 0;
    ranges[pid].var_count = 0;
    ranges[pid].pcb_start = -1;
    ranges[pid].inst_start = -1;
    ranges[pid].var_start = -1;

    printf("Freed memory for P%d: %d words\n", pid, total_words_freed);
}
void resetMemory()
{
    // Free all memory words
    freeMemoryWord();
    freeIndex(&index_table);
    resetMemoryRanges();
    current_memory_usage = 0;
}

void resetMemoryRanges()
{
    for (int i = 0; i < MAX_NUM_PROCESSES; i++)
    {
        ranges[i].pid = -1;
        ranges[i].pcb_count = 0;
        ranges[i].inst_count = 0;
        ranges[i].var_count = 0;
        ranges[i].pcb_start = -1;
        ranges[i].inst_start = -1;
        ranges[i].var_start = -1;
    }
    ranges_count = 0;
}