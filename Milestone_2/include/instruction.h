#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include "process.h"
#include "memory_manager.h"
#include "mutex.h"
#include "console_model.h"

extern mutex_t userInput_mutex;
extern mutex_t userOutput_mutex;
extern mutex_t file_mutex;

#define MAX_VAR_KEY_LEN 15
#define MAX_ARG_LEN 100

// Function declarations

// Input
char *input(const char *prompt);

// Print
void print(Process *process, char *printable);

// Assign with given value
void assign(Process *process, char *arg1, char *arg2);

// Write string to file
void writeToFile(Process *process, char *varFileName, char *varContent);

// Read string from file
char *readFromFile(Process *process, char *varFileName);

// Print range of numbers
void printFromTo(Process *process, char *arg1, char *arg2);

// Semaphore functions
void semWait(Process *process, char *resource);
void semSignal(Process *process, char *resource);

#endif // INSTRUCTIONS_H
