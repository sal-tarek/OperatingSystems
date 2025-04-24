#ifndef PARSER_H
#define PARSER_H

#include "memory.h"
#include "process.h"
#include "PCB.h"
#include "../include/instruction.h"

// Constants
#define MAX_NAME_LEN 50     // Maximum length for instruction/variable strings
#define MAX_VAR_COUNT 3     // Number of variables per process
#define DECODE_HASH_SIZE 7  // Number of instructions for decoding hashmap
#define EXECUTE_HASH_SIZE 7 // Number of instructions for execution hashmap

// Enum for instruction types
typedef enum
{
    PRINT,
    ASSIGN,
    WRITETOFILE,
    READFILE,
    PRINTFROMTO,
    SEMWAIT,
    SEMSIGNAL
} InstructionType;

// Instruction structure for decoded instructions
typedef struct
{
    InstructionType type;    // Instruction type (e.g., PRINT)
    char arg1[MAX_NAME_LEN]; // First argument (e.g., "x")
    char arg2[MAX_NAME_LEN]; // Second argument (e.g., "input")
} Instruction;

// Decoding hashmap entry (maps string to instruction type)
typedef struct
{
    const char *key;       // Instruction string (e.g., "print")
    InstructionType value; // Instruction type (e.g., PRINT)
} DecodeHashEntry;

// Function prototypes
// Fetching
char* fetch_instruction(PCB* pcb, int pid);

// Decoding
Instruction decode_instruction(char* instruction_string);

// Execution - Main function
void execute_instruction(PCB* pcb, int pid, Instruction* instruction);

// Execution Cycle
void exec_cycle(int pid);

#endif // PARSER_H
