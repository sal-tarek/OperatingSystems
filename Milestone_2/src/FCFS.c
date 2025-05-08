#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"
#include "parser.h"

// runs one cycle of the FCFS scheduler
void runFCFS()
{
    // Display the ready queues
    printf("Before ");
    for (int i = 0; i < 4; i++)
        displayQueueSimplified(readyQueues[i]);
    printf("Blocked ");
    displayQueueSimplified(global_blocked_queue);
    
    // Fetch the next process from the ready queue & Handling Blocked processes
    if (!isEmpty(readyQueues[0]))
        runningProcess = peek(readyQueues[0]);

    // Update the timeInQueue for all processes in the ready queue
    Process *temp = readyQueues[0]->front;
    while (temp != NULL)
    {
        temp->timeInQueue++;
        temp = temp->next;
    }

    if (runningProcess != NULL)
    {
        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING;

        exec_cycle(runningProcess); // Execution of the next instruction of the process

        runningProcess->remainingTime--;
        printf("Executed Process %d remaining time: %d time in queue: %d\n", runningProcess->pid, runningProcess->remainingTime, runningProcess->timeInQueue);

        if(runningProcess->state != BLOCKED)
        {
            // Check if the process has finished
            if (runningProcess->remainingTime == 0)
            {
                dequeue(readyQueues[0]); // Now we safely remove it from the queue
                setProcessState(runningProcess->pid, TERMINATED);
                runningProcess->state = TERMINATED;
                printf("Process %d finished execution at time %d\n", runningProcess->pid, clockCycle);

                deleteProcessFromMemory(runningProcess->pid); // Free the memory allocated for the process
                numberOfProcesses--;
                
                runningProcess = NULL;
            }
            else
            {
                setProcessState(runningProcess->pid, READY);
                runningProcess->state = READY;
            }
        }
    }
    else
    {
        printf("CPU is idle\n");
    }
    
    // Display the ready queues
    printf("After ");
    for (int i = 0; i < 4; i++)
        displayQueueSimplified(readyQueues[i]);
    printf("Blocked ");
    displayQueueSimplified(global_blocked_queue);
}