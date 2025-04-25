#ifndef MLFQ_H
#define MLFQ_H

#include "Queue.h"
#include "process.h"
#include "parser.h"

#define numProcesses 3
#define numQueues 4

extern Queue *readyQueues[numQueues];
extern Process *runningProcess;
extern int clockCycle;

// Function to run the Multilevel Feedback Queue scheduler
void runMLFQ();

#endif 