#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RoundRobin.h"

// runs one cycle of the RR scheduler
void runRR(int quantum) {     
    printf("\nTime %d: \n \n", clockCycle);

    // Print ready queue
    printf("Ready ");
    displayQueueSimplified(readyQueues[0]); 

    // Fetch the next process from the ready queue & Handling Blocked processes
    while (!isEmpty(readyQueues[0])) {
        if(peek(readyQueues[0])->state == WAITING) 
            dequeue(readyQueues[0]); 
        else{
            runningProcess = peek(readyQueues[0]);
            break;
        }
    }
        
    if(runningProcess != NULL){
        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING;

        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        // Execute(current->pid);              // Simulate the execution of the process
        printf("Executing %d\n", runningProcess->pid);

        if(runningProcess->remainingTime == 0) {
            dequeue(readyQueues[0]);  // Now we safely remove it from the queue
            printf("Finished %d\n", runningProcess->pid);
            setProcessState(runningProcess->pid, TERMINATED);
        }else if(runningProcess->quantumUsed == quantum){
            dequeue(readyQueues[0]);  
            enqueue(readyQueues[0], runningProcess); 
            printf("moved %d level 0\n", runningProcess->pid);
            
            runningProcess->quantumUsed = 0; 
            runningProcess = NULL; 
        }
        else{ 
            setProcessState(runningProcess->pid, READY);
            runningProcess->state = READY;
        }
    }
    else{
        printf("CPU is idle\n", clockCycle);
    }                     
}