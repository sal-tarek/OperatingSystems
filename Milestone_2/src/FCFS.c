#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"

void runFCFS() {
    printf("\nTime %d: \n \n", clockCycle);

    // Fetch the next process from the ready queue & Handling Blocked processes
    while (!isEmpty(readyQueues[0])) {
        if(peek(readyQueues[0])->state == BLOCKED) 
            dequeue(readyQueues[0]); 
        else{
            runningProcess = peek(readyQueues[0]);
            exec_cycle(runningProcess); // Execution of the next instruction of the process

            if(runningProcess->state == BLOCKED) {
                dequeue(readyQueues[0]); // Remove the process from the queue
                runningProcess = NULL; // Clear runningProcess
            }
            else
                break;
        }
    }

    // Print ready queue
    printf("Ready ");
    displayQueueSimplified(readyQueues[0]);

    // If a process is running, execute it
    if (runningProcess != NULL) {
        setProcessState(runningProcess->pid, RUNNING); 
        runningProcess->state = RUNNING;
        
        exec_cycle(runningProcess); // Simulate the execution of the process
        printf("Executing %d\n", runningProcess->pid);

        runningProcess->remainingTime--;
        printf("Process %d remaining time: %d\n", runningProcess->pid, runningProcess->remainingTime);

        // Check if the process has finished
        if (runningProcess->remainingTime == 0) {
            dequeue(readyQueues[0]); // Remove the process from the queue
            setProcessState(runningProcess->pid, TERMINATED); 
            printf("Process %d finished execution at time %d\n", runningProcess->pid, clockCycle);
            runningProcess = NULL; // Clear runningProcess
        }
        else{ 
            setProcessState(runningProcess->pid, READY);
            runningProcess->state = READY; 
        }
    }
}