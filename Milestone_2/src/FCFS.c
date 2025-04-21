#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"

void runFCFS() {
    printf("\nTime %d: \n \n", clockCycle);

    // Print ready queue
    printf("Ready ");
    displayQueueSimplified(readyQueues[0]);

    // If no process is running, try to start the next process from the ready queue
    if (!isEmpty(readyQueues[0])) {
        runningProcess = peek(readyQueues[0]);
    }
    // If a process is running, execute it
    if (runningProcess != NULL) {
        setProcessState(runningProcess->pid, RUNNING); 
        runningProcess->remainingTime--;
        printf("Process %d remaining time: %d\n", runningProcess->pid, runningProcess->remainingTime);

        // Check if the process has finished
        if (runningProcess->remainingTime == 0) {
            dequeue(readyQueues[0]); // Remove the process from the queue
            setProcessState(runningProcess->pid, TERMINATED); 
            printf("Process %d finished execution at time %d\n", runningProcess->pid, clockCycle);
            runningProcess = NULL; // Clear runningProcess
        }
        else setProcessState(runningProcess->pid, READY);
    }
}