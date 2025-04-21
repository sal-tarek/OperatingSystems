#include "../include/instruction.h"
#include "../include/mutex.h"
#include <string.h>
#include <errno.h> // This is where EBUSY is defined

// Define the actual mutexes
mutex_t fileMutex;
mutex_t inputMutex;
mutex_t outputMutex;

#define MAX_VAR_KEY_LEN 15
#define MAX_ARG_LEN 100

// Helper functions

// Take input from user
char *input(char *functionality) {
    char  *output;

    printf("Please %s\n", functionality);
    scanf("%s", output);

    return output;
}

// Detects if the token is `input`
int isInput(const char* token) {
    return strcmp(token, "input") == 0;
}

// Detects if the token starts with "readFile"
int isReadFile(const char* token) {
    return strncmp(token, "readFile ", 9) == 0;
}

// Extracts filename from "readFile filename"
char* extractFileName(const char* token) {
    char* fileName = malloc(MAX_ARG_LEN);
    sscanf(token, "readFile %[^\n]", fileName);
    return fileName;
}

// Main Functions

// Print "printStatement" to terminal
void print(char *printStatement)
{
    printf("%s", printStatement);
}

#define MAX_TOKENS 4

// Assigns value to a variable 
// value and variable could be provided directly, read from a file, or read from terminal
void assign(Process *process, char *args) {
    char *tokens[MAX_TOKENS];
    int tokenCount = 0;

    char buffer[MAX_ARG_LEN];
    strncpy(buffer, args, MAX_ARG_LEN);

    char *token = strtok(buffer, " ");
    while (token && tokenCount < MAX_TOKENS) {
        if (isReadFileStart(token)) {
            char *next = strtok(NULL, " ");
            if (!next) {
                fprintf(stderr, "Error: readFile command missing filename.\n");
                return;
            }
            tokens[tokenCount++] = mergeReadFileToken(token, next);
        } else {
            tokens[tokenCount++] = strdup(token);
        }
        token = strtok(NULL, " ");
    }

    if (tokenCount != 2) {
        fprintf(stderr, "Error: assign expects exactly 2 arguments.\n");
        for (int i = 0; i < tokenCount; i++) free(tokens[i]);
        return;
    }

    char *arg1 = tokens[0];
    char *arg2 = tokens[1];
    char *variable = NULL;
    char *value = NULL;

    if (strcmp(arg1, "input") == 0) {
        variable = arg2;
        value = input("Enter value: ");
    } else if (strncmp(arg1, "readFile ", 9) == 0) {
        variable = arg2;
        value = readFromFile(arg1 + 9);
    } else if (strcmp(arg2, "input") == 0) {
        variable = arg1;
        value = input("Enter value: ");
    } else if (strncmp(arg2, "readFile ", 9) == 0) {
        variable = arg1;
        value = readFromFile(arg2 + 9);
    } else {
        variable = arg1;
        value = arg2;
    }

    if (!variable || !value) {
        fprintf(stderr, "Error: failed to resolve variable or value.\n");
        goto cleanup;
    }

    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, variable);

    updateDataByIndex(index, memory, varKey, value, "TYPE_STRING");

cleanup:
    for (int i = 0; i < tokenCount; i++) free(tokens[i]);
}

// Write string to file
void writeToFile(char *filename, char *content)
{
    FILE *fptr = fopen(filename, "w");

    if (fptr == NULL)
    {
        perror("Error opening file");
        return;
    }

    fprintf(fptr, "%s", content);
    fclose(fptr);
}

// Read file content and return as string
char *readFromFile(char *fileName)
{
    if (strcmp(fileName, "input") == 0) {
        fileName = input("enter a file name");
    }

    FILE *fptr = fopen(fileName, "r");

    if (fptr == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    rewind(fptr);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        perror("Failed to allocate memory");
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, fptr);
    buffer[bytes_read] = '\0';

    fclose(fptr);
    return buffer;
}

// parses the input to get 2 integers and print the numbers between them (inclusive)
void printFromTo(char *args)
{
    char *tokens[MAX_TOKENS];
    int tokenCount = 0;

    char buffer[MAX_ARG_LEN];
    strncpy(buffer, args, MAX_ARG_LEN);

    char *token = strtok(buffer, " ");
    while (token && tokenCount < MAX_TOKENS) {
        if (isReadFileStart(token)) {
            char *next = strtok(NULL, " ");
            if (!next) {
                fprintf(stderr, "Error: readFile command missing filename.\n");
                return;
            }
            tokens[tokenCount++] = mergeReadFileToken(token, next);
        } else {
            tokens[tokenCount++] = strdup(token);
        }
        token = strtok(NULL, " ");
    }

    if (tokenCount != 2) {
        fprintf(stderr, "Error: assign expects exactly 2 arguments.\n");
        for (int i = 0; i < tokenCount; i++) free(tokens[i]);
        return;
    }

    char *arg1 = tokens[0];
    char *arg2 = tokens[1];
    char *firstInt = NULL;
    char *secondInt = NULL;

    if (strcmp(arg1, "input") == 0) {
        firstInt = arg2;
        secondInt = input("enter value: ");
    } else if (strncmp(arg1, "readFile ", 9) == 0) {
        firstInt = arg2;
        secondInt = readFromFile(arg1 + 9);
    } else if (strcmp(arg2, "input") == 0) {
        firstInt = arg1;
        secondInt = input("enter value: ");
    } else if (strncmp(arg2, "readFile ", 9) == 0) {
        firstInt = arg1;
        secondInt = readFromFile(arg2 + 9);
    } else {
        firstInt = arg1;
        secondInt = arg2;
    }

    if (!firstInt || !secondInt) {
        fprintf(stderr, "Error: failed to resolve first integer or second integer.\n");
        goto cleanup;
    }

    int x = atoi(firstInt), y = atoi(secondInt);

    for (int i = x; i <= y; i++)
    {
        printf("%d", i);
        if (i != y) printf(" ");
    }
    printf("\n");

cleanup:
    for (int i = 0; i < tokenCount; i++) free(tokens[i]);
}

// Semaphore wait function (locks a mutex)
/*
void semWait(char *x)
{
    int result = 0;
    if (strcmp(x, "file") == 0) {
        // Lock the file mutex
        result = mutex_lock(&fileMutex);
        printf("semWait called on file\n");
    } else if (strcmp(x, "userInput") == 0) {
        result = mutex_lock(&inputMutex);  // Lock the input mutex
        printf("semWait called on user input\n");
    } else if (strcmp(x, "userOutput") == 0) {
        result = mutex_lock(&outputMutex);  // Lock the output mutex
        printf("semWait called on user output\n");
    } else {
        perror("invalid resource\n");
        return;
    }

    if (result == 0) {
        // Mutex was successfully locked (it wasn't locked before)
        printf("Mutex was not locked, now locked.\n");
    } else if (result == EBUSY) {
        // Mutex was already locked by another thread
        printf("Mutex is already locked by another process.\n");
    } else {
        // Handle other error cases
        perror("pthread_mutex_trylock");
    }
}

// Semaphore signal function (unlocks a mutex)
void semSignal(char *x) {
    if (strcmp(x, "file") == 0) {
        // Lock the file mutex
        result = mutex_unlock(&fileMutex);
        printf("semSignal called on file\n");
    } else if (strcmp(x, "userInput") == 0) {
        result = mutex_unlock(&inputMutex);  // Lock the input mutex
        printf("semSignal called on user input\n");
    } else if (strcmp(x, "userOutput") == 0) {
        result = mutex_unlock(&outputMutex);  // Lock the output mutex
        printf("semSignal called on user output\n");
    } else {
        perror("invalid resource\n");
        return;
    }
}
    */