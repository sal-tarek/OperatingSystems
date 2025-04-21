#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "Queue.h"
#include "FCFS.h"

const char* processStateToString(ProcessState state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case WAITING: return "WAITING";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}
void runFCFS() {
    printf("=== FCFS Scheduling Simulation at time %d ===\n", clockCycle);

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
            printf("Process state %s\n", processStateToString(runningProcess->state));
            free(runningProcess); // Free the process
            runningProcess = NULL; // Clear runningProcess
        }
    }
}