#include "parser.h"
#include <string.h>
#include <stdlib.h>

// Static decoding hashmap
static DecodeHashEntry decode_hashmap[DECODE_HASH_SIZE] = {
{"print", PRINT},
{"assign", ASSIGN},
{"writeFile", WRITEFILE},
{"readFile", READFILE},
{"printFromTo", PRINTFROMTO},
{"semWait", SEMWAIT},
{"semSignal", SEMSIGNAL}
};

// Static execution hashmap
static ExecuteHashEntry execute_hashmap[EXECUTE_HASH_SIZE] = {
{PRINT, exec_print},
{ASSIGN, exec_assign},
{WRITEFILE, exec_write_file},
{READFILE, exec_read_file},
{PRINTFROMTO, exec_print_from_to},
{SEMWAIT, exec_sem_wait},
{SEMSIGNAL, exec_sem_signal}
};

// Fetch instruction string from memory
char *fetch_instruction(MemoryWord *memory, PCB *pcb, Process *process) {
    // Check if programCounter exceeds burstTime
    if (pcb->programCounter >= process->burstTime) {
        return NULL;
    }

    // Compute memory address
    int address = pcb->memory_base_address + 1 + pcb->programCounter;

    // Look up address in memory hash table
    MemoryWord *word = NULL;
    HASH_FIND_INT(memory, &address, word);

    // Return instruction string or NULL if not found
    return word ? word->data : NULL;
}

// Decode instruction string into Instruction struct
Instruction decode_instruction(char *instruction_string) {
Instruction result = {0}; // Initialize with zeros

// Create a copy to avoid modifying the original string
char *copy = strdup(instruction_string);

// Tokenize the string
char *token = strtok(copy, " ");

// Look up command in decoding hashmap
for (int i = 0; i < DECODE_HASH_SIZE; i++) {
if (strcmp(token, decode_hashmap[i].key) == 0) {
result.type = decode_hashmap[i].value;
break;
}
}

// Extract arguments based on instruction type
if (result.type == PRINT || result.type == SEMWAIT || result.type == SEMSIGNAL || result.type == READFILE) {
// One argument 
token = strtok(NULL, " ");
if (token) {
strncpy(result.arg1, token, MAX_NAME_LEN - 1);
result.arg1[MAX_NAME_LEN - 1] = '\0';
}
} else if (result.type == ASSIGN || result.type == WRITEFILE || result.type == PRINTFROMTO) {
// Two arguments 
token = strtok(NULL, " ");
if (token) {
strncpy(result.arg1, token, MAX_NAME_LEN - 1);
result.arg1[MAX_NAME_LEN - 1] = '\0';
token = strtok(NULL, " ");
if (token) {
strncpy(result.arg2, token, MAX_NAME_LEN - 1);
result.arg2[MAX_NAME_LEN - 1] = '\0';
}
}
}
free(copy);
return result;
}

// Execute instruction and update process state
void execute_instruction(MemoryWord *memory, PCB *pcb, Process *process, Instruction *instruction) {
    // Find and call syntax function
    for (int i = 0; i < EXECUTE_HASH_SIZE; i++) {
        if (execute_hashmap[i].key == instruction->type) {
            execute_hashmap[i].handler(pcb, instruction);
            break;
        }
    }

    // Update process state
    pcb->programCounter++;
    process->remainingTime--;

    if (process->remainingTime <= 0 || pcb->programCounter >= process->burstTime) {
        pcb->state = TERMINATED;
        process->state = TERMINATED;
    }
}

void exec_print(PCB *pcb, Instruction *instr)
{}
void exec_assign(PCB *pcb, Instruction *instr)
{}
void exec_write_file(PCB *pcb, Instruction *instr)
{}
void exec_read_file(PCB *pcb, Instruction *instr)
{}
void exec_print_from_to(PCB *pcb, Instruction *instr)
{}
void exec_sem_wait(PCB *pcb, Instruction *instr)
{}
void exec_sem_signal(PCB *pcb, Instruction *instr)
{}



/* Helper to print Instruction struct
void print_instruction(Instruction instr) {
    printf("Type: %d, Arg1: %s, Arg2: %s\n", instr.type, instr.arg1, instr.arg2);
}

 int main() {
    // Test cases
    char *tests[] = {
        "print x",
        "assign x input",
        "writeFile x y",
        "readFile x",
        "printFromTo x y",
        "semWait userInput",
        "semSignal userOutput"
    };
    int num_tests = 7;

    // Expected results
    InstructionType expected_types[] = {PRINT, ASSIGN, WRITEFILE, READFILE, PRINTFROMTO, SEMWAIT, SEMSIGNAL};
    char *expected_arg1[] = {"x", "x", "x", "x", "x", "userInput", "userOutput"};
    char *expected_arg2[] = {"", "input", "y", "", "y", "", ""};

    // Run tests
    for (int i = 0; i < num_tests; i++) {
        printf("Testing: %s\n", tests[i]);
        Instruction result = decode_instruction(tests[i]);
        print_instruction(result);

        // Verify results
        int pass = result.type == expected_types[i] &&
                   strcmp(result.arg1, expected_arg1[i]) == 0 &&
                   strcmp(result.arg2, expected_arg2[i]) == 0;
        printf("Test %d: %s\n\n", i + 1, pass ? "PASS" : "FAIL");
    }

    return 0;
}*/
