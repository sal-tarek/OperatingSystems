#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Declare the global mutexes (extern = defined elsewhere) -- to be implemented
/*
    extern mutex fileMutex;
    extern mutex inputMutex;
    extern mutex outputMutex;
*/

// Function declarations

// Print string
void printStr(char *x);

// Print integer
void printInt(int x);

// Assign string
void assignStr(char **x, char *y);

// Assign integer
void assignInt(int *x, int y);

// Assign integer from user input
void assignInput(int *x);

// Write string to file
int writeToFile(char *filename, char *content);

// Read string from file
char* readFromFile(char *filename);

// Print range of numbers
void printFromTo(int x, int y);

// Semaphore functions
void semWait(char *x);
void semSignal(char *x);

#endif // INSTRUCTIONS_H
