#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../src/process.h"
#include "../src/memory_manager.h"

// Declare the global mutexes (extern = defined elsewhere) -- to be implemented
/*
    extern mutex fileMutex;
    extern mutex inputMutex;
    extern mutex outputMutex;
*/

// Function declarations

// Print
void print(int processId, char *printable);

// Assign with given value
void assign(int processId, char *arg1, char *arg2);

// Write string to file
void writeFile(char *fileName, char *content);

// Read string from file
char *readFromFile(char *fileName);

// Print range of numbers
void printFromTo(int processId, char *arg1, char *arg2);

// Semaphore functions
void semWait(char *resource);
void semSignal(char *resource);

#endif // INSTRUCTIONS_H
