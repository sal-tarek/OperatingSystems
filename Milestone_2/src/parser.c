#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "instruction.h"

// Static decoding hashmap
static DecodeHashEntry decode_hashmap[DECODE_HASH_SIZE] = {
    {"print", PRINT},
    {"assign", ASSIGN},
    {"writeToFile", WRITETOFILE},
    {"readFile", READFILE},
    {"printFromTo", PRINTFROMTO},
    {"semWait", SEMWAIT},
    {"semSignal", SEMSIGNAL}};

// Fetch the current instruction for a process based on its PCB's program counter
char *fetch_instruction(PCB *pcb, int pid)
{
    // Validate PID and get memory range
    MemoryRange r = getProcessMemoryRange(pid);
    /*
    if (r.inst_count == 0 && r.var_count == 0 && r.pcb_count == 0)
    {
        printf("debug: 1");
        return NULL; // Error already logged by getProcessMemoryRange
    }
        
    // Check if program counter exceeds instruction count
    if (pcb->programCounter >= r.inst_count)
    {
        printf("debug: 2");
        return NULL; // No more instructions to fetch
    }
        */
    // Generate key for the current instruction
    char key[32];
    snprintf(key, sizeof(key), "P%d_Instruction_%d", pid, pcb->programCounter + 1);

    // Fetch the instruction from memory using the index
    DataType type;
    char *instruction = fetchDataByIndex(key, &type);
    if (instruction == NULL)
    {
        console_log_printf("Failed to fetch instruction for key: %s\n", key);
        return NULL;
    }

    // Verify the data type is a string
    if (type != TYPE_STRING)
    {
        console_log_printf("Invalid data type for instruction key %s: expected TYPE_STRING, got %d\n", key, type);
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
        console_log_printf("Error: Memory allocation failed for instruction copy\n");
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

    // Validate argument count based on instruction type
    if (result.type == PRINT || result.type == SEMWAIT || result.type == SEMSIGNAL || result.type == READFILE)
    {
        // Expect exactly 1 argument
        if (tokenCount != 2)
        {
            console_log_printf("Error: %s expects exactly 1 argument, got %d\n", tokens[0], tokenCount - 1);
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
            console_log_printf("Error: %s expects exactly 2 arguments, got %d\n", tokens[0], tokenCount - 1);
            free(copy);
            return result;
        }
        // Handle arg1
        if (strcmp(tokens[1], "input") == 0)
        {
            char *userInput = input("Enter value for arg1: ");
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
            char *userInput = input("Enter value for arg2: ");
            if (!userInput)
            {
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
        console_log_printf("Error: Unknown instruction type: %s\n", tokens[0]);
    }

    free(copy);
    return result;
}

void trim_trailing_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
}

// Execute instruction using the new handler functions and increment PC using the passed PCB
void execute_instruction(PCB *pcb, Process* process, Instruction *instruction)
{
    if (!instruction)
    {
        console_log_printf("Error: Invalid instruction\n");
        return;
    }
    if (!pcb)
    {
        console_log_printf("Error: PCB is NULL for process %d\n", process->pid);
        return;
    }
    if (!process)
    {
        console_log_printf("Error: process is NULL\n");
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
        // Already handled in decode_instruction, but we can log the result
        console_log_printf("readFile result for process %d: %s\n", process->pid, instruction->arg1);
        readFromFile(process, instruction->arg1);
        break;
    case PRINTFROMTO:
        printFromTo(process, instruction->arg1, instruction->arg2);
        break;
    case SEMWAIT:
        // printf("Debugging: Resource being used: %s\n", instruction->arg1);
        // printf("Debugging: instruction type: %p\n", process);
        // printf("Debugging: prcoess accessing test: %d", process->state);
        semWait(process, instruction->arg1);
        break;
    case SEMSIGNAL:
        semSignal(process, instruction->arg1);
        break;
    default:
        console_log_printf("Error: Unknown instruction type: %d\n", instruction->type);
        return;
    }
    // Increment the program counter using the passed PCB
    pcb->programCounter += 1;
}

// Execution cycle: Fetch, decode, and execute an instruction for a given process
void exec_cycle(Process* process)
{
    // Format the PCB key as P<pid>_PCB
    char pcb_key[32]; // Sufficient size for "P<pid>_PCB"
    snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", process->pid);

    // Fetch the PCB using fetchDataByIndex
    DataType type;
    void *data = fetchDataByIndex(pcb_key, &type);
    if (!data || type != TYPE_PCB)
    {
        console_log_printf("Error: Failed to fetch PCB for process %d (key: %s)\n", process->pid, pcb_key);
        return;
    }

    PCB *pcb = (PCB *)data;

    console_log_printf("Debugging: process accessing test: %d  program counter %d\n", process->pid, pcb->programCounter);
    // Fetch instruction
    char *instruction_str = fetch_instruction(pcb, process->pid);
    // printf("Debugging: memory dump after fetch\n");
    // printMemory();
    if (!instruction_str)
    {
        console_log_printf("Error: Failed to fetch instruction for process %d\n", process->pid);
        return;
    }

    // Decode instruction
    Instruction instruction = decode_instruction(process, instruction_str);
    // printf("Debugging: memory dump after decode\n");
    // printMemory();
    if (instruction.arg1[0] == '\0' && instruction.arg2[0] == '\0')
    {
        console_log_printf("Error: Failed to decode instruction for process %d: %s\n", process->pid, instruction_str);
        // free(instruction_str);
        return;
    }

    // Execute instruction (PC will be incremented inside execute_instruction)
    // printf("Debugging: prcoess accessing test: %d\n", process->pid);
    execute_instruction(pcb, process, &instruction);
    // printf("Debugging: memory dump after execute\n");
    // printMemory();

    // Clean up
    // free(instruction_str);
}

// Minimal main function to test decode_instruction and execute_instruction
/*
int main() {
    printf("Starting parser test...\n");

    // Create a mock PCB
    PCB mock_pcb = { 0 };
    mock_pcb.id = 1;              // Process ID
    mock_pcb.programCounter = 0;   // Start at instruction 0
    mock_pcb.state = RUNNING;      // Process must be in RUNNING state

    // Mock instructions (simulating a .txt file)
    char* instructions[] = {
    "assign h hello",
    "print h",         // Print "hello"
    "assign d data",
    "writeToFile file1 d", // Write "data" to "file1"
    };
    int num_instructions = 4;

    // Process each instruction
    for (int i = 0; i < num_instructions; i++) {
        printf("Processing instruction %d: %s\n", i + 1, instructions[i]);

        // Decode the instruction
        Instruction decoded = decode_instruction(Process *process, instructions[i]);
        if (decoded.type == 0 && decoded.arg1[0] == '\0' && decoded.arg2[0] == '\0') {
            printf("Failed to decode instruction: %s\n", instructions[i]);
            continue;
        }

        // Execute the instruction
        execute_instruction(&mock_pcb, mock_pcb.id, &decoded);
        printf("Program Counter after execution: %d\n", mock_pcb.programCounter);
    }

    printf("Parser test completed.\n");
    return 0;
}
    */

/*
Starting parser test...
Processing instruction 1: print hello
Process 0 prints: hello
Program Counter after execution: 1
Processing instruction 2: assign x 5
Process 0 assigns: x = 5
Program Counter after execution: 2
Processing instruction 3: writeToFile file1 data
Wrote to file file1: data
Program Counter after execution: 3
Processing instruction 4: printFromTo 1 3
Process 0 prints range: 1 2 3
Program Counter after execution: 4
Parser test completed.
*/