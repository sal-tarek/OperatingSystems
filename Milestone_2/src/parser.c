#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "instruction.h"

// Static decoding hashmap
static DecodeHashEntry decode_hashmap[DECODE_HASH_SIZE] = {
    {"print", PRINT},
    {"assign", ASSIGN},
    {"writeFile", WRITETOFILE},  // Add alias for writeFile
    {"readFile", READFILE},
    {"printFromTo", PRINTFROMTO},
    {"semWait", SEMWAIT},
    {"semSignal", SEMSIGNAL}};

// Fetch the current instruction for a process based on its PCB's program counter
char *fetch_instruction(PCB *pcb, int pid)
{
    // Generate key for the current instruction
    char key[32];
    snprintf(key, sizeof(key), "P%d_Instruction_%d", pid, pcb->programCounter + 1);

    // Fetch the instruction from memory using the index
    DataType type;
    char *instruction = fetchDataByIndex(key, &type);
    if (instruction == NULL)
    {
        console_model_log_output("[ERROR] Failed to fetch instruction for process %d at PC=%d\n",
                                 pid, pcb->programCounter);
        return NULL;
    }

    // Verify the data type is a string
    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for instruction key %s: expected STRING, got %d\n",
                                 key, type);
        return NULL;
    }

    return instruction;
}

// Decode instruction string into Instruction struct
Instruction decode_instruction(Process *process, char *instruction_string)
{
    Instruction result = {0}; // Initialize with zeros
    char *tokens[3];          // Max 3 tokens: command, arg1, arg2
    int tokenCount = 0;

    // Create a copy to avoid modifying the original string
    char *copy = strdup(instruction_string);
    if (!copy)
    {
        console_model_log_output("[ERROR] Memory allocation failed for instruction copy\n");
        return result;
    }

    // Tokenize the string
    char *token = strtok(copy, " ");
    while (token && tokenCount < 3)
    {
        tokens[tokenCount++] = token;
        token = strtok(NULL, " ");
    }

    // Look up command in decoding hashmap
    if (tokenCount > 0)
    {
        for (int i = 0; i < DECODE_HASH_SIZE; i++)
        {
            if (strcmp(tokens[0], decode_hashmap[i].key) == 0)
            {
                result.type = decode_hashmap[i].value;
                break;
            }
        }
    }

    // Log the instruction being decoded
    console_model_log_output("[EXEC] Decoding instruction: %s\n", instruction_string);

    // Validate argument count based on instruction type
    if (result.type == PRINT || result.type == SEMWAIT || result.type == SEMSIGNAL || result.type == READFILE)
    {
        // Expect exactly 1 argument
        if (tokenCount != 2)
        {
            console_model_log_output("[ERROR] Invalid number of arguments for '%s': expected 1, got %d\n", 
                                    tokens[0], tokenCount - 1);
            free(copy);
            return result;
        }
        // Special handling for readFile
        if (result.type == READFILE)
        {
            strncpy(result.arg1, tokens[1], MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
            char *fileContent = readFromFile(process, result.arg1);
            if (!fileContent)
            {
                free(copy);
                return result;
            }
            strncpy(result.arg1, fileContent, MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
            free(fileContent);
        }
        else
        {
            strncpy(result.arg1, tokens[1], MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
        }
    }
    else if (result.type == ASSIGN || result.type == WRITETOFILE || result.type == PRINTFROMTO)
    {
        // Expect exactly 2 arguments
        if (tokenCount != 3)
        {
            console_model_log_output("[ERROR] Invalid number of arguments for '%s': expected 2, got %d\n", 
                                    tokens[0], tokenCount - 1);
            free(copy);
            return result;
        }
        // Handle arg1
        if (strcmp(tokens[1], "input") == 0)
        {
            char prompt[256];
            switch (result.type) {
                case ASSIGN:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter value to assign to variable '%s': \n", process->pid, tokens[2]);
                    break;
                case WRITETOFILE:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter content to write to file '%s': \n", process->pid, tokens[2]);
                    break;
                case PRINTFROMTO:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter start value for range: \n", process->pid);
                    break;
                default:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter value for first argument: \n", process->pid);
            }
            char *userInput = input(prompt);
            if (!userInput)
            {
                free(copy);
                return result;
            }
            strncpy(result.arg1, userInput, MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
            free(userInput);
        }
        else if (strncmp(tokens[1], "readFile", 8) == 0)
        {
            char *filename = tokens[1] + 9; // Skip "readFile "
            char *fileContent = readFromFile(process, filename);
            if (!fileContent)
            {
                free(copy);
                return result;
            }
            strncpy(result.arg1, fileContent, MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
            free(fileContent);
        }
        else
        {
            strncpy(result.arg1, tokens[1], MAX_NAME_LEN - 1);
            result.arg1[MAX_NAME_LEN - 1] = '\0';
        }
        // Handle arg2
        if (strcmp(tokens[2], "input") == 0)
        {
            char prompt[256];
            switch (result.type) {
                case ASSIGN:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter value to assign to variable '%s': \n", process->pid, tokens[1]);
                    break;
                case WRITETOFILE:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter content to write to file '%s': \n", process->pid, tokens[1]);
                    break;
                case PRINTFROMTO:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter end value for range: \n", process->pid);
                    break;
                default:
                    snprintf(prompt, sizeof(prompt), "Process %d: Enter value for second argument: \n", process->pid);
            }
            char *userInput = input(prompt);
            
            if (!userInput)
            {
                console_model_log_output("[ERROR] Failed to get user input for arg2\n");
                free(copy);
                return result;
            }
            strncpy(result.arg2, userInput, MAX_NAME_LEN - 1);
            result.arg2[MAX_NAME_LEN - 1] = '\0';
            free(userInput);
        }
        else if (strncmp(tokens[2], "readFile", 8) == 0)
        {
            char *filename = tokens[2] + 9; // Skip "readFile "
            char *fileContent = readFromFile(process, filename);
            if (!fileContent)
            {
                free(copy);
                return result;
            }
            strncpy(result.arg2, fileContent, MAX_NAME_LEN - 1);
            result.arg2[MAX_NAME_LEN - 1] = '\0';
            free(fileContent);
        }
        else
        {
            strncpy(result.arg2, tokens[2], MAX_NAME_LEN - 1);
            result.arg2[MAX_NAME_LEN - 1] = '\0';
        }
    }
    else
    {
        console_model_log_output("[ERROR] Unknown instruction: '%s'\n", tokens[0]);
    }

    free(copy);
    return result;
}

void trim_trailing_whitespace(char *str)
{
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1]))
    {
        str[--len] = '\0';
    }
}

// Execute instruction using the new handler functions and increment PC using the passed PCB
void execute_instruction(PCB *pcb, Process *process, Instruction *instruction)
{
    if (!instruction)
    {
        console_model_log_output("[ERROR] Invalid instruction pointer\n");
        return;
    }
    if (!pcb)
    {
        console_model_log_output("[ERROR] PCB is NULL for process %d\n", process->pid);
        return;
    }
    if (!process)
    {
        console_model_log_output("[ERROR] Process pointer is NULL\n");
        return;
    }

    trim_trailing_whitespace(instruction->arg1);
    trim_trailing_whitespace(instruction->arg2);

    // Execute the instruction
    switch (instruction->type)
    {
    case PRINT:
        print(process, instruction->arg1);
        break;
    case ASSIGN:
        assign(process, instruction->arg1, instruction->arg2);
        break;
    case WRITETOFILE:
        writeToFile(process, instruction->arg1, instruction->arg2);
        break;
    case READFILE:
        readFromFile(process, instruction->arg1);
        break;
    case PRINTFROMTO:
        printFromTo(process, instruction->arg1, instruction->arg2);
        break;
    case SEMWAIT:
        semWait(process, instruction->arg1);
        break;
    case SEMSIGNAL:
        semSignal(process, instruction->arg1);
        break;
    default:
        console_model_log_output("[ERROR] Unknown instruction type: %d\n", instruction->type);
        return;
    }
    // Increment the program counter using the passed PCB
    pcb->programCounter += 1;
}

// Execution cycle: Fetch, decode, and execute an instruction for a given process
void exec_cycle(Process *process)
{
    // Format the PCB key as P<pid>_PCB
    char pcb_key[32]; // Sufficient size for "P<pid>_PCB"
    snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", process->pid);

    // Fetch the PCB using fetchDataByIndex
    DataType type;
    void *data = fetchDataByIndex(pcb_key, &type);
    if (!data || type != TYPE_PCB)
    {
        console_model_log_output("[ERROR] Failed to fetch PCB for process %d\n", process->pid);
        return;
    }

    PCB *pcb = (PCB *)data;

    // Log the start of execution cycle
    console_model_log_output("[EXEC] Process %d executing instruction at PC=%d\n", process->pid, pcb->programCounter);

    // Fetch instruction
    char *instruction_str = fetch_instruction(pcb, process->pid);
    if (!instruction_str)
    {
        console_model_log_output("[ERROR] Failed to fetch instruction for process %d at PC=%d\n", 
                                process->pid, pcb->programCounter);
        return;
    }

    // Decode instruction
    Instruction instruction = decode_instruction(process, instruction_str);
    if (instruction.type == 0 && instruction.arg1[0] == '\0' && instruction.arg2[0] == '\0')
    {
        console_model_log_output("[ERROR] Failed to decode instruction for process %d: '%s'\n",
                                process->pid, instruction_str);
        return;
    }

    // Execute instruction (PC will be incremented inside execute_instruction)
    execute_instruction(pcb, process, &instruction);

    // Log the completion of execution cycle
    console_model_log_output("[EXEC] Process %d completed instruction at PC=%d\n", process->pid, pcb->programCounter - 1);
}