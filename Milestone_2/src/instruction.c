#include <string.h>
#include <errno.h>
#include <limits.h>
#include <gtk/gtk.h> // Add GTK/GLib headers
#include "instruction.h"
#include "mutex.h"

#define MAX_VAR_KEY_LEN 15
#define MAX_ARG_LEN 100

// Helper function signatures
int safe_atoi(const char *str, int *out);

// Main Functions

char *input(const char *prompt)
{
    console_model_log_output("[INPUT] User input requested: %s\n", prompt);

    // Request input from the user via console model
    char *user_input = console_model_request_input(prompt);

    // Check if input is NULL to avoid segmentation fault
    if (!user_input)
    {
        console_model_log_output("[ERROR] Input request failed\n");
        return g_strdup(""); // Return empty string instead of NULL
    }

    // Remove newline character if present
    size_t len = strlen(user_input);
    if (len > 0 && user_input[len - 1] == '\n')
    {
        user_input[len - 1] = '\0';
    }

    console_model_log_output("[INPUT] User entered: \"%s\"\n", user_input);
    return user_input;
}

// Print "printStatement" to terminal
void print(Process *process, char *printable)
{
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, printable);
    char *storedData = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for variable '%s'\n", printable);
        return;
    }

    if (storedData != NULL)
    {
        console_model_program_output("Output of program with process ID: %d\n%s\n", process->pid, storedData);
        console_model_log_output("[OUTPUT] Process %d printed variable '%s' with value '%s'\n",
                                 process->pid, printable, storedData);
    }
    else
    {
        console_model_program_output("Output of program with process ID: %d\nVariable not found!\n", process->pid);
        console_model_log_output("[ERROR] Process %d tried to print non-existent variable '%s'\n",
                                 process->pid, printable);
    }
}

// Assigns value to a variable
void assign(Process *process, char *arg1, char *arg2)
{
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, arg1);

    updateDataByIndex(varKey, arg2, TYPE_STRING);
    console_model_log_output("[MEMORY] Process %d: Variable '%s' assigned value '%s'\n",
                             process->pid, arg1, arg2);
}

// Write string to file
void writeToFile(Process *process, char *varfileName, char *varContent)
{
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, varfileName);
    char *fileName = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for variable '%s'\n", varfileName);
        return;
    }

    FILE *fptr = fopen(fileName, "w");

    if (fptr == NULL)
    {
        console_model_log_output("[ERROR] Process %d failed to create file '%s'\n", process->pid, fileName);
        return;
    }

    console_model_program_output("Output of program with process ID: %d\nFile %s created successfully.\n", process->pid, fileName);
    console_model_log_output("[FILE] Process %d created file '%s' and wrote %lu bytes\n",
                             process->pid, fileName, strlen(varContent));
    fprintf(fptr, "%s", varContent);
    fclose(fptr);
}

// Read file content and return as string
char *readFromFile(Process *process, char *varfileName)
{
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, varfileName);
    char *fileName = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for variable '%s'\n", varfileName);
        return NULL;
    }

    FILE *fptr = fopen(fileName, "r");

    if (fptr == NULL)
    {
        console_model_log_output("[ERROR] Process %d failed to open file '%s' for reading\n",
                                 process->pid, fileName);
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    rewind(fptr);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        console_model_log_output("[ERROR] Process %d failed to allocate memory for file '%s' content\n",
                                 process->pid, fileName);
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, fptr);
    buffer[bytes_read] = '\0';

    console_model_log_output("[FILE] Process %d read %lu bytes from file '%s'\n",
                             process->pid, bytes_read, fileName);
    fclose(fptr);
    return buffer;
}

// parses the input to get 2 integers and print the numbers between them (inclusive)
void printFromTo(Process *process, char *arg1, char *arg2)
{
    DataType type;
    char varKey[MAX_VAR_KEY_LEN];
    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, arg1);
    char *storedData1 = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for variable '%s'\n", arg1);
        return;
    }

    snprintf(varKey, MAX_VAR_KEY_LEN, "P%d_Variable_%s", process->pid, arg2);
    char *storedData2 = (char *)fetchDataByIndex(varKey, &type);

    if (type != TYPE_STRING)
    {
        console_model_log_output("[ERROR] Invalid data type for variable '%s'\n", arg2);
        return;
    }

    int x, y, errCode1 = safe_atoi(storedData1, &x), errCode2 = safe_atoi(storedData2, &y);

    if (errCode1 == 0 && errCode2 == 0)
    {
        if (x > y)
        {
            console_model_log_output("[ERROR] printFromTo: first argument (%d) is greater than second argument (%d)\n", x, y);
            return;
        }
        console_model_program_output("Output of program with process ID: %d\n", process->pid);
        for (int i = x; i <= y; i++)
        {
            console_model_program_output("%d", i);
            if (i != y)
                console_model_program_output(" ");
        }
        console_model_program_output("\n");
        console_model_log_output("[OUTPUT] Process %d executed printFromTo from %d to %d\n", process->pid, x, y);
    }
    else if (errCode1 == 1 || errCode2 == 1)
    {
        console_model_log_output("[ERROR] printFromTo: invalid numeric input (not a number): '%s' or '%s'\n",
                                 storedData1, storedData2);
    }
    else
    {
        console_model_log_output("[ERROR] printFromTo: value out of int range: '%s' or '%s'\n",
                                 storedData1, storedData2);
    }
}

// Semaphore wait function (locks a mutex)
void semWait(Process *process, char *x)
{
    int result = 0;
    if (strcmp(x, "file") == 0)
    {
        // Lock the file mutex
        result = mutex_lock(&file_mutex, process);
        console_model_log_output("[MUTEX] Process %d attempting to acquire file resource\n", process->pid);
    }
    else if (strcmp(x, "userInput") == 0)
    {
        result = mutex_lock(&userInput_mutex, process); // Lock the input mutex
        console_model_log_output("[MUTEX] Process %d attempting to acquire userInput resource\n", process->pid);
    }
    else if (strcmp(x, "userOutput") == 0)
    {
        result = mutex_lock(&userOutput_mutex, process); // Lock the output mutex
        console_model_log_output("[MUTEX] Process %d attempting to acquire userOutput resource\n", process->pid);
    }
    else
    {
        console_model_log_output("[ERROR] Process %d tried to acquire an invalid resource: %s\n", process->pid, x);
        return;
    }

    if (result != 0)
    {
        // Process was blocked
        console_model_log_output("[BLOCKED] Process %d blocked waiting for resource: %s\n", process->pid, x);
    }
    else
    {
        console_model_log_output("[MUTEX] Process %d successfully acquired resource: %s\n", process->pid, x);
    }
}

// Semaphore signal function (unlocks a mutex)
void semSignal(Process *process, char *x)
{
    if (strcmp(x, "file") == 0)
    {
        // Unlock the file mutex
        mutex_unlock(&file_mutex, process);
        console_model_log_output("[MUTEX] Process %d released file resource\n", process->pid);
    }
    else if (strcmp(x, "userInput") == 0)
    {
        mutex_unlock(&userInput_mutex, process); // Unlock the input mutex
        console_model_log_output("[MUTEX] Process %d released userInput resource\n", process->pid);
    }
    else if (strcmp(x, "userOutput") == 0)
    {
        mutex_unlock(&userOutput_mutex, process); // Unlock the output mutex
        console_model_log_output("[MUTEX] Process %d released userOutput resource\n", process->pid);
    }
    else
    {
        console_model_log_output("[ERROR] Process %d tried to release an invalid resource: %s\n",
                                 process->pid, x);
        return;
    }

    // Check if any process was unblocked
    Process *unblocked = checkUnblocked(x);
    if (unblocked)
    {
        console_model_log_output("[UNBLOCKED] Process %d was unblocked from resource %s\n",
                                 unblocked->pid, x);
    }
}

// Helper Functions implementation

// parses a string to an int
// returns 0 on success, 1 on invalid input (not a number), and 2 if out of int range
int safe_atoi(const char *str, int *out)
{
    if (str == NULL || *str == '\0')
    {
        // Invalid string input
        return 1;
    }

    char *end;
    errno = 0; // clear errno before call

    long value = strtol(str, &end, 10);

    if (end == str || *end != '\0')
    {
        // No digits found, or extra characters after the number
        return 1;
    }

    if ((value == LONG_MAX || value == LONG_MIN) && errno == ERANGE)
    {
        // Overflow or underflow
        return 2;
    }

    if (value > INT_MAX || value < INT_MIN)
    {
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
                fprintf(stderr, "Error: readFile command missing fileName.\n");
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