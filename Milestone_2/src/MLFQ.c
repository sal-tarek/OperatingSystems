#include <stdio.h>
#include <stdlib.h>
#include "MLFQ.h"               

int lastUsedLevel = -1;  // -1 means no pending process

// runs one cycle of the MLFQ scheduler
void runMLFQ() {
    printf("\nTime %d: \n \n", clockCycle);

    // Display the ready queues
    for(int i = 0; i < 4; i++)
        displayQueueSimplified(readyQueues[i]);


    // Find the highest priority process in the queues if no pending process & handling Blocked processes
    if(lastUsedLevel == -1){
        for (int i = 0; i < numQueues; i++) {
            while (!isEmpty(readyQueues[i])) {
                if(peek(readyQueues[i])->state == WAITING) 
                    dequeue(readyQueues[i]); 
                else{
                    runningProcess = peek(readyQueues[i]);
                    lastUsedLevel = i;
                    break;
                }
            }
            if(runningProcess != NULL) break; // Exit if we found a process
        }
    }

    if(runningProcess != NULL){
        int timeQuantum = 1 << lastUsedLevel; // time quantum = 2^i for queue i {Queue 0 --> 1, Queue 1 --> 2, Queue 2 --> 4, Queue 3 --> 8}

        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING; 

        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        // Execute(current->pid);              // Simulate the execution of the process
        printf("Executing %d\n", runningProcess->pid);

        if(runningProcess->remainingTime == 0) {
            dequeue(readyQueues[lastUsedLevel]);  // Now we safely remove it from the queue
            printf("Finished %d\n", runningProcess->pid);
            setProcessState(runningProcess->pid, TERMINATED);
            runningProcess->state = TERMINATED;
            runningProcess->quantumUsed = 0; 
            runningProcess = NULL; 
            lastUsedLevel = -1;
        }else if(runningProcess->quantumUsed == timeQuantum){
            dequeue(readyQueues[lastUsedLevel]);  
            if(lastUsedLevel != 3) {
                enqueue(readyQueues[lastUsedLevel+1], runningProcess); // Move to next queue
                setProcessPriority(runningProcess->pid, getProcessPriority(runningProcess->pid)+1); // Increase priority
                printf("moved %d Level %d\n", runningProcess->pid, lastUsedLevel+2);
            }
            else{
                enqueue(readyQueues[lastUsedLevel], runningProcess); // Stay in the same queue (Last Queue - RR)
                printf("moved %d Level %d\n", runningProcess->pid, lastUsedLevel+1);
            }
            runningProcess->quantumUsed = 0; 
            runningProcess = NULL; 
            lastUsedLevel = -1;
        }
        else{
            setProcessState(runningProcess->pid, READY);
            runningProcess->state = READY;
        }
    }
    else{
        printf("CPU is idle\n", clockCycle);
    }

    return;
}