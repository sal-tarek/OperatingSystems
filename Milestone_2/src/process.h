#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef struct Process {
    int pid;                // Process ID (e.g., "P1")
    ProcessState state;       // Current state of the process
    char *file_path;          // Path to the file (e.g., "p1.txt")
    int arrival_time;         // Time the process arrives
    int ready_time;           // Time the process becomes ready
    int burstTime;            // Total execution time required
    int remainingTime;        // Time left to execute
    struct Process *next;     // For queue linked list
} Process;

Process* createProcess(const char *pid, const char *file_path, int arrival_time, int burst_time);

#endif // PROCESS_H
