#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../src/process.h"


// Declare the global mutexes (extern = defined elsewhere) -- to be implemented
/*
    extern mutex fileMutex;
    extern mutex inputMutex;
    extern mutex outputMutex;
*/

// Function declarations

// Print
void print(char* printStatement);

// Assign with given value
void assignValue(Process process, char* userProvidedValue);

// Assign with user Input
void assignInput(Process process);

// Write string to file
int writeFile(char *filename, char *content);

// Read string from file
void readFromFile(char *filename);

// Print range of numbers
void printFromTo(int firstInt, int secondInt);

// Semaphore functions
void semWait(char *x);
void semSignal(char *x);

#endif // INSTRUCTIONS_H
