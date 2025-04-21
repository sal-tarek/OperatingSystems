#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../src/process.h"
#include "memory_manager.h"

extern MemoryWord *memory; // Will store address-to-data mappings
extern IndexEntry *index;  // Will store instruction-to-address mappings

// Declare the global mutexes (extern = defined elsewhere) -- to be implemented
/*
    extern mutex fileMutex;
    extern mutex inputMutex;
    extern mutex outputMutex;
*/

// Function declarations

// Print
void print(char *printStatement);

// Assign with given value
void assign(Process *process, char *args);

// Write string to file
void writeFile(char *fileName, char *content);

// Read string from file
char *readFromFile(char *fileName);

// Print range of numbers
void printFromTo(char *args);

// Semaphore functions
void semWait(char *x);
void semSignal(char *x);

#endif // INSTRUCTIONS_H
