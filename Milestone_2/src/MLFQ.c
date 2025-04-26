#include <stdio.h>
#include <stdlib.h>
#include "MLFQ.h"               

int lastUsedLevel = -1;  // -1 means no pending process

// runs one cycle of the MLFQ scheduler
void runMLFQ() {
    printf("\nTime %d: \n \n", clockCycle);

    // Find the highest priority process in the queues if no pending process & handling Blocked processes
    if(lastUsedLevel != -1){
        // check if this process is still/ will be not blocked
        if(runningProcess->state == BLOCKED) {
            dequeue(readyQueues[lastUsedLevel]);
            runningProcess = NULL; // Reset running process
            lastUsedLevel = -1;
        }

        exec_cycle(runningProcess); // Execution of the next instruction of the process

        if(runningProcess->state == BLOCKED) {
            dequeue(readyQueues[lastUsedLevel]);
            runningProcess = NULL; // Reset running process
            lastUsedLevel = -1;
        }
    }
    if(lastUsedLevel == -1){
        for (int i = 0; i < numQueues; i++) {
            while (!isEmpty(readyQueues[i])) {
                if(peek(readyQueues[i])->state == BLOCKED) 
                    dequeue(readyQueues[i]);
                else{
                    runningProcess = peek(readyQueues[i]);
                    lastUsedLevel = i;

                    exec_cycle(runningProcess); // Execution of the next instruction of the process

                    if(runningProcess->state == BLOCKED) {
                        dequeue(readyQueues[i]);
                        runningProcess = NULL; // Reset running process
                    }
                }
                if(runningProcess != NULL) break; // Exit if we found a process
            }
            if(runningProcess != NULL) break; // Exit if we found a process
        }
    }

    // Display the ready queues
    for(int i = 0; i < 4; i++)
        displayQueueSimplified(readyQueues[i]);

    if(runningProcess != NULL){
        int timeQuantum = 1 << lastUsedLevel; // time quantum = 2^i for queue i {Queue 0 --> 1, Queue 1 --> 2, Queue 2 --> 4, Queue 3 --> 8}

        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING; 

        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        printf("Executed %d\n", runningProcess->pid);

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
                printf("moving %d to Level %d next clock cycle\n", runningProcess->pid, lastUsedLevel+2);
            }
            else{
                enqueue(readyQueues[lastUsedLevel], runningProcess); // Stay in the same queue (Last Queue - RR)
                printf("moving %d to Level %d\n next clock cycle", runningProcess->pid, lastUsedLevel+1);
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
        printf("CPU is idle\n");
        //printf("CPU is idle\n", clockCycle); where is the placeholder??
    }

    return;
}