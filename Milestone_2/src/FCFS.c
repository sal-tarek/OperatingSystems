#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"
#include "parser.h"

// runs one cycle of the FCFS scheduler
void runFCFS() {
    printf("\nTime %d: \n \n", clockCycle);

    // Fetch the next process from the ready queue & Handling Blocked processes
    while (!isEmpty(readyQueues[0])) {
        if(peek(readyQueues[0])->state == BLOCKED) 
            dequeue(readyQueues[0]); 
        else{
            runningProcess = peek(readyQueues[0]);
            exec_cycle(runningProcess); 

            if(runningProcess->state == BLOCKED) {
                dequeue(readyQueues[0]); 
                runningProcess = NULL; // Clear runningProcess
            }
            else
                break;
        }
    }

    // Update the timeInQueue for all processes in the ready queue
    Process *temp = readyQueues[0]->front;
    while (temp != NULL) {
        temp->timeInQueue++;
        temp = temp->next;
    }

    // If a process is running, execute it
    if (runningProcess != NULL) {
        setProcessState(runningProcess->pid, RUNNING); 

        runningProcess->state = RUNNING;
        runningProcess->remainingTime--;
        printf("Executed Process %d remaining time: %d time in queue: %d\n", runningProcess->pid, runningProcess->remainingTime, runningProcess->timeInQueue);

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
    else{
        printf("CPU is idle\n", clockCycle);
    }

    // Print ready queue
    printf("Ready ");
    displayQueueSimplified(readyQueues[0]);
}