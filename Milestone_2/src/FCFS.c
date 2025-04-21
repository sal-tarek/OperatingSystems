#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"

void runFCFS() {
    printf("\n=== FCFS Scheduling Simulation at time %d ===\n", clockCycle);
    // Print state of the ready queue
    printf("Ready Queue: ");
    display(readyQueues[0]);
    printf("\n");

    // If no process is running, try to start the next process from the ready queue
    if (!isEmpty(readyQueues[0])) {
        runningProcess = peek(readyQueues[0]);
    }
    // If a process is running, execute it
    if (runningProcess != NULL) {
        runningProcess->state = RUNNING;
        runningProcess->remainingTime--;
        printf("Process %d remaining time: %d\n", runningProcess->pid, runningProcess->remainingTime);

        // Check if the process has finished
        if (runningProcess->remainingTime == 0) {
            dequeue(readyQueues[0]); // Remove the process from the queue
            runningProcess->state = TERMINATED;
            printf("Process %d finished execution at time %d\n", runningProcess->pid, clockCycle);
            runningProcess = NULL; // Clear runningProcess
        }
    }
}