#include <string.h>
#include <errno.h>
#include <limits.h>
#include "instruction.h"

// Helper function signatures
int safe_atoi(const char *str, int *out);

// Main Functions

char *input(const char *prompt)
{
    console_printf("%s", prompt);
    static char buffer[MAX_ARG_LEN];
    console_scanf(buffer, MAX_ARG_LEN);
    buffer[strcspn(buffer, "\n")] = '\0';
    console_printf("> %s\n", buffer);
    return strdup(buffer);
}

// Print "printStatement" to terminal
void print(int processId, char *printable)
{
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, printable);
    char *storedData = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING) {
        perror("Erroneous fetch\n");
        return;
    }

    if (storedData != NULL) {
        console_printf("%s\n", storedData);
    } else {
        console_printf("Variable not found!\n");
    }
}

#define MAX_TOKENS 4

// Assigns value to a variable
void assign(int processId, char *arg1, char *arg2) {
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, arg1);

    updateDataByIndex(varKey, arg2, TYPE_STRING);
}

// Write string to file
void writeToFile(char *filename, char *content)
{
    FILE *fptr = fopen(filename, "w");

    if (fptr == NULL)
    {
        console_printf("Failed to create the file\n");
        return;
    }

    console_printf("File %s created successfully.\n", filename);
    fprintf(fptr, "%s", content);
    fclose(fptr);
}

// Read file content and return as string
char *readFromFile(char *fileName)
{
    FILE *fptr = fopen(fileName, "r");

    if (fptr == NULL)
    {
        console_printf("Error opening file\n");
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    rewind(fptr);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        console_printf("Failed to allocate memory\n");
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
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, arg1);
    char *storedData1 = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING) {
        console_printf("Erroneous fetch\n");
        return;
    }

    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", processId, arg2);
    char *storedData2 = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING) {
        console_printf("Erroneous fetch\n");
        return;
    }

    int x, y, errCode1 = safe_atoi(storedData1, &x), errCode2 = safe_atoi(storedData2, &y);

    if (errCode1 == 0 && errCode2 == 0) {
        if (x > y) {
            console_printf("second argument is smaller than first argument");
            return;
        }
        
        for (int i = x; i <= y; i++)
        {
            console_printf("%d", i);
            if (i != y) console_printf(" ");
        }
        printf("\n");
    } else if (errCode1 == 1 || errCode2 == 1) {
        console_printf("either values are invalid inputs (not a number)");
    } else {
        console_printf("either values is out of int range");
    }
}

// Semaphore wait function (locks a mutex)
void semWait(Process* process, char *x)
{
    int result = 0;
    if (strcmp(x, "file") == 0) {
        // Lock the file mutex
        result = mutex_lock(&file_mutex, process);
        console_printf("semWait called on file\n");
    } else if (strcmp(x, "userInput") == 0) {
        result = mutex_lock(&userInput_mutex, process);  // Lock the input mutex
        console_printf("semWait called on user input\n");
    } else if (strcmp(x, "userOutput") == 0) {
        result = mutex_lock(&userOutput_mutex, process);  // Lock the output mutex
        console_printf("semWait called on user output\n");
    } else {
        console_printf("invalid resource\n");
        return;
    }

    // if (result == 0) {
    //     // Mutex was successfully locked (it wasn't locked before)
    //     printf("Mutex was not locked, now locked.\n");
    // } else {
    //     // Mutex was already locked by another thread
    //     printf("Mutex is already locked by another process.\n");
    // }
}

// Semaphore signal function (unlocks a mutex)
void semSignal(Process* process, char *x) {
    if (strcmp(x, "file") == 0) {
        // Lock the file mutex
        mutex_unlock(&file_mutex, process);
        console_printf("semSignal called on file\n");
    } else if (strcmp(x, "userInput") == 0) {
        mutex_unlock(&userInput_mutex, process);  // Unlock the input mutex
        console_printf("semSignal called on user input\n");
    } else if (strcmp(x, "userOutput") == 0) {
        mutex_unlock(&userOutput_mutex, process);  // Unlock the output mutex
        console_printf("semSignal called on user output\n");
    } else {
        console_printf("invalid resource\n");
        return;
    }
}

// Helper Functions implementation

// parses a string to an int
// returns 0 on success, 1 on invalid input (not a number), and 2 if out of int range
int safe_atoi(const char *str, int *out) {
    if (str == NULL || *str == '\0') {
        // Invalid string input
        return 1;
    }

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