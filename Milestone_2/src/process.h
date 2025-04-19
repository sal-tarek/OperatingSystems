#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
    char *pid;          // Process ID (e.g., "P1")
    char *file_path;    // Path to text file (e.g., "p1.txt")
    int arrival_time;   // Arrival time (e.g., 0 for P1)
} Process;

#endif // PROCESS_H