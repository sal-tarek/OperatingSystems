#ifndef FCFS_H
#define FCFS_H

#include "Queue.h"
#include "process.h"
#include "parser.h"

#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support
#define MAX_NUM_QUEUES 4        // Maximum number of queues

extern Queue *readyQueues[MAX_NUM_QUEUES];
extern Process *runningProcess;
extern int clockCycle;
extern int numProcesses;

// Function to run the First Come First Serve scheduler
void runFCFS();

#endif 