#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <instruction.h>

// Static decoding hashmap
static DecodeHashEntry decode_hashmap[DECODE_HASH_SIZE] = {
    {"print", PRINT},
    {"assign", ASSIGN},
    {"writeFile", WRITEFILE},
    {"readFile", READFILE},
    {"printFromTo", PRINTFROMTO},
    {"semWait", SEMWAIT},
    {"semSignal", SEMSIGNAL}};

// Static execution hashmap
static ExecuteHashEntry execute_hashmap[EXECUTE_HASH_SIZE] = {
    {PRINT, print},
    {ASSIGN, assign},
    {WRITEFILE, writeFile},
    {READFILE, readFile},
    {PRINTFROMTO, printFromTo},
    {SEMWAIT, semWait},
    {SEMSIGNAL, semSignal}
};

// Fetch the current instruction for a process based on its PCB's program counter
char *fetch_instruction(MemoryWord *memory, IndexEntry *index, PCB *pcb, Process *process)
{
    // Validate PID and get memory range
    MemoryRange range = getProcessMemoryRange(process->pid);
    if (range.inst_count == 0 && range.var_count == 0 && range.pcb_count == 0)
    {
        fprintf(stderr, "Invalid PID %d in fetch_instruction\n", process->pid);
        return NULL;
    }
    // Check if program counter exceeds instruction count
    if (pcb->programCounter >= range.inst_count)
    {
        return NULL; // No more instructions to fetch
    }
    // Generate key for the current instruction
    char key[32];
    snprintf(key, sizeof(key), "P%d_Instruction_%d", process->pid, pcb->programCounter + 1);
    // Fetch the instruction from memory using the index
    DataType type;
    char *instruction = fetchDataByIndex(key, &type);
    if (instruction == NULL)
    {
        fprintf(stderr, "Failed to fetch instruction for key: %s\n", key);
        return NULL;
    }
    // Verify the data type is a string
    if (type != TYPE_STRING)
    {
        fprintf(stderr, "Invalid data type for instruction key %s: expected TYPE_STRING, got %d\n", key, type);
        return NULL;
    }
    return instruction;
}

// Decode instruction string into Instruction struct
Instruction decode_instruction(char *instruction_string)
{
    Instruction result = {0}; // Initialize with zeros

    // Create a copy to avoid modifying the original string
    char *copy = strdup(instruction_string);

    // Tokenize the string
    char *token = strtok(copy, " ");

    // Look up command in decoding hashmap
    for (int i = 0; i < DECODE_HASH_SIZE; i++)
    {
        if (strcmp(token, decode_hashmap[i].key) == 0)
        {
            result.type = decode_hashmap[i].value;
            break;
        }
    }

    // Extract arguments based on instruction type
    if (result.type == PRINT || result.type == SEMWAIT || result.type == SEMSIGNAL || result.type == READFILE)
    {
        // One argument
        token = strtok(NULL, " ");
        if (token)
        {
            strncpy(result.arg1, token, MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
        }
    }
    else if (result.type == ASSIGN || result.type == WRITEFILE || result.type == PRINTFROMTO)
    {
        // Two arguments
        token = strtok(NULL, " ");
        if (token)
        {
            strncpy(result.arg1, token, MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
            token = strtok(NULL, " ");
            if (token)
            {
                strncpy(result.arg2, token, MAX_NAME_LEN - 1);
                result.arg2[MAX_NAME_LEN - 1] = '\0';
            }
        }
    }
    free(copy);
    return result;
}

// Execute instruction and update process state
void execute_instruction(MemoryWord *memory, PCB *pcb, Process *process, Instruction *instruction)
{
    // Execute based on instruction type
    switch (instruction->type)
    {
    case PRINT:
        print(pcb->id, instruction->arg1);
        break;
    case ASSIGN:
        assign(pcb->id, &instruction->arg1, instruction->arg2);
        break;
    case WRITEFILE:
        writeToFile(instruction->arg1, instruction->arg2);
        break;
    case READFILE:
        // Note: Result is not stored; may need memory update
        (void)readFromFile(instruction->arg1);
        break;
    case PRINTFROMTO:
        printFromTo(pcb->id, instruction->arg1, instruction->arg2);
        break;
    case SEMWAIT:
        semWait(instruction->arg1);
        // Assume semWait sets pcb->state to WAITING if blocked
        break;
    case SEMSIGNAL:
        semSignal(instruction->arg1);
        break;
    default:
        // Invalid instruction type
        break;
    }

    // Update program counter and remaining time
    pcb->programCounter++;
    process->remainingTime--; //

    // Check for process termination
    if (process->remainingTime <= 0 || pcb->programCounter >= process->burstTime)
    {
        pcb->state = TERMINATED;
    }
}