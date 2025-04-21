#include <stdio.h>
#include <stdlib.h>
#include "MLFQ.h"               

// runs one cycle of the MLFQ scheduler
void runMLFQ() {
    printf("\nTime %d: \n \n", clockCycle);

    // Display the ready queues
    display(readyQueues[0]);
    display(readyQueues[1]);
    display(readyQueues[2]);
    display(readyQueues[3]);

    int level = -1;

    // Find the highest priority existing process in the queues
    for (int i = 0; i < numQueues; i++) {
        if (!isEmpty(readyQueues[i])) {
            runningProcess = peek(readyQueues[i]);
            level = i;
            break;
        }
    }
    

    if(runningProcess != NULL){
        int timeQuantum = 1 << level; // time quantum = 2^i for queue i {Queue 0 --> 1, Queue 1 --> 2, Queue 2 --> 4, Queue 3 --> 8}

        runningProcess->state = RUNNING;
        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        // Execute(current->pid);              // Simulate the execution of the process
        printf("Executing %d\n", runningProcess->pid);

        if(runningProcess->remainingTime == 0) {
            dequeue(readyQueues[level]);  // Now we safely remove it from the queue
            printf("Finished %d\n", runningProcess->pid);
            runningProcess->state = TERMINATED;
        }else if(runningProcess->quantumUsed == timeQuantum){
            dequeue(readyQueues[level]);  
            if(level != 3) {
                enqueue(readyQueues[level+1], runningProcess); // Move to next queue
                printf("moved %d level %d\n", runningProcess->pid, level+2);
            }
            else{
                enqueue(readyQueues[level], runningProcess); // Stay in the same queue (Last Queue - RR)
                printf("moved %d level %d\n", runningProcess->pid, level+1);
            }
            runningProcess->state = READY;
            runningProcess->quantumUsed = 0; 
            runningProcess = NULL; 
        }
    }else{
        printf("CPU is idle\n", clockCycle);
    }

    return;
}