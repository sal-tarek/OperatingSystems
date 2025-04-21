#include <string.h>
#include <errno.h>
#include <limits.h>
#include "../include/instruction.h"
#include "../include/mutex.h"

// Define the actual mutexes
mutex_t fileMutex;
mutex_t inputMutex;
mutex_t outputMutex;

#define MAX_VAR_KEY_LEN 15
#define MAX_ARG_LEN 100

// Helper function signatures
safe_atoi(const char *str, int *out);
char *input(char *functionality);

// Main Functions

// Print "printStatement" to terminal
void print(int processId, char *printable)
{
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, 1);
    char *storedData1 = (char *)fetchDataByIndex(varKey, "TYPE_STRING");

    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, 2);
    char *storedData2 = (char *)fetchDataByIndex(varKey, "TYPE_STRING");

    char *varName1, varName2, varValue1, varValue2;
    sscanf(storedData1, "%99[^:]:%99[^\n]", varName1, varValue1);
    sscanf(storedData2, "%99[^:]:%99[^\n]", varName2, varValue2);

    if (strcmp(printable, varName1) == 0) {
        printf("%s", varValue1);
    } else if (strcmp(printable, varName2) == 0) {
        printf("%s", varValue2);
    } else {
        perror("Variable not found!");
    }
}

#define MAX_TOKENS 4

// Assigns value to a variable 
// value could be provided directly, read from a file, or read from terminal
void assign(int processId, char *arg1, char *arg2) {
    char *value = NULL;

    if (strcmp(arg2, "input") == 0) {
        value = input("enter a value to store in variable");
    } else if (strncmp(arg2, "readFile ", 9) == 0) {
        value = readFromFile(arg2 + 9);
    } else {
        value = arg2;
    }

    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, (strcmp(arg1, "a") == 0? 1 : 2));

    char *store;
    snprintf(store, 1024, "%s:%s", arg1, value);
    updateDataByIndex(varKey, store, "TYPE_STRING");

    printf(store);
}

// Write string to file
void writeToFile(char *filename, char *content)
{
    FILE *fptr = fopen(filename, "w");

    if (fptr == NULL)
    {
        perror("Failed to create the file");
        return;
    }

    printf("File created successfully.\n");
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
void printFromTo(int processId, char *arg1, char *arg2)
{
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, 1);
    char *storedData1 = (char *)fetchDataByIndex(varKey, "TYPE_STRING");

    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, 2);
    char *storedData2 = (char *)fetchDataByIndex(varKey, "TYPE_STRING");

    char *varName1, varName2, varValue1, varValue2;
    sscanf(storedData1, "%99[^:]:%99[^\n]", varName1, varValue1);
    sscanf(storedData2, "%99[^:]:%99[^\n]", varName2, varValue2);

    int x, y, errCode1 = 2, errCode2 = 2;
    if (strcmp(arg1, varName1) == 0) {
        errCode1 = safe_atoi(arg1, x);
        errCode2 = safe_atoi(arg2, y);
    } else {
        errCode1 = safe_atoi(arg1, y);
        errCode2 = safe_atoi(arg2, x);
    }

    if (errCode1 == 0 && errCode2 == 0) {
        if (y > x) {
            printf("second argument is smaller than first argument");
            return;
        }
        
        for (int i = x; i <= y; i++)
        {
            printf("%d", i);
            if (i != y) printf(" ");
        }
        printf("\n");
    } else if (errCode1 == 1 || errCode2 == 1) {
        perror("either values are invalid inputs (not a number)");
    } else {
        perror("either values is out of int range");
    }
}

// Semaphore wait function (locks a mutex)
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
    } else {
        // Mutex was already locked by another thread
        printf("Mutex is already locked by another process.\n");
    }
}

// Semaphore signal function (unlocks a mutex)
void semSignal(char *x) {
    if (strcmp(x, "file") == 0) {
        // Lock the file mutex
        mutex_unlock(&fileMutex);
        printf("semSignal called on file\n");
    } else if (strcmp(x, "userInput") == 0) {
        mutex_unlock(&inputMutex);  // Unlock the input mutex
        printf("semSignal called on user input\n");
    } else if (strcmp(x, "userOutput") == 0) {
        mutex_unlock(&outputMutex);  // Unlock the output mutex
        printf("semSignal called on user output\n");
    } else {
        perror("invalid resource\n");
        return;
    }
}

// Helper Functions implementation

// parses a string to an int
// returns 0 on success, 1 on invalid input (not a number), and 2 if out of int range
int safe_atoi(const char *str, int *out) {
    char *end;
    errno = 0; // clear errno before call

    long value = strtol(str, &end, 10);

    if (end == str || *end != '\0') {
        // No digits found, or extra characters after the number
        return 1;
    }

    if ((value == LONG_MAX || value == LONG_MIN) && errno == ERANGE) {
        // Overflow or underflow
        return 2;
    }

    if (value > INT_MAX || value < INT_MIN) {
        // Out of int range
        return 2;
    }

    *out = (int)value;
    return 0;
}

// Take input from user
char *input(char *functionality) {
    char  *output;

    printf("Please %s\n", functionality);
    scanf("%s", output);

    return output;
}

/* Code that might be used later:
    char *tokens[MAX_TOKENS];
    int tokenCount = 0;

    char buffer[MAX_ARG_LEN];
    strncpy(buffer, args, MAX_ARG_LEN);

    char *token = strtok(buffer, " ");
    while (token && tokenCount < MAX_TOKENS) {
        if (isReadFile(token)) {
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

    
cleanup:
    for (int i = 0; i < tokenCount; i++) free(tokens[i]);
*/