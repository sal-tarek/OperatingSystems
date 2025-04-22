#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

typedef struct Process {
    int pid;                // Process ID (e.g., 1 for P1)
    char *file_path;        // Path to file (e.g., "../programs/Program_1.txt")
    int arrival_time;       // Time the process arrives
    int ready_time;         // Time the process becomes ready
    int burstTime;          // Total execution time required
    int remainingTime;      // Time left to execute
    //struct PCB *pcb;        // Pointer to the PCB structure
    struct Process *next;   // For queue linked list
} Process;

Process* createProcess(int pid, const char *file_path, int arrival_time, int burstTime);
void displayProcess(Process *p);
void freeProcess(Process *p);

#endif // PROCESS_H
