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

// Assign
void assignValue(Process process, char* userProvidedValue);

// Assign integer from user input
void assignInput(int *x)
{
    printf("Please enter a value: ");
    scanf("%d", x);
}

// Write string to file
int writeToFile(char *filename, char *content)
{
    FILE *fptr = fopen(filename, "w");

    if (fptr == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    fprintf(fptr, "%s", content);
    fclose(fptr);

    return 0;
}

// Read file content and return as string
char *readFromFile(char *filename)
{
    FILE *fptr = fopen(filename, "r");

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

// Print numbers from x to y
void printFromTo(int x, int y)
{
    for (int i = x; i <= y; i++)
    {
        printf("%d ", i);
    }
    printf("\n");
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