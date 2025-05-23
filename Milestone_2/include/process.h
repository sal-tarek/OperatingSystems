#ifndef PROCESS_H
#define PROCESS_H

typedef enum { NEW, READY, RUNNING, BLOCKED, TERMINATED, ERROR } ProcessState;


typedef struct Process {
    int pid;                // Process ID (e.g., 1 for P1)
    char *file_path;        // Path to file (e.g., "../programs/Program_1.txt")
    ProcessState state;     // Process state (NEW, READY, RUNNING, WAITING, TERMINATED)
    int arrival_time;       // Time the process arrives
    int ready_time;         // Time the process becomes ready
    int burstTime;          // Total execution time required
    int remainingTime;      // Time left to execute
    int quantumUsed;        // number of cycles used in the current quantum assigned to this process
    int timeInQueue;        // Tracks how many cycles this process waits in the Ready Queue
    struct Process *next;   // For queue linked list
    int instruction_count; // Number of instructions in the program
    char *instructions;     // string of instructions
    int variable_count;    // Number of variables in the program
    char **variables;     // Array of variable names
} Process;

Process *createProcess(int pid, const char *file_path, int arrival_time);
void displayProcess(Process *p);
void freeProcess(Process *p);
char* getProcessInstructions(int pid);
ProcessState getProcessState(int pid);
void setProcessState(int pid, ProcessState newState);
int getProcessPriority(int pid);
void setProcessPriority(int pid, int priority);

#endif // PROCESS_H
