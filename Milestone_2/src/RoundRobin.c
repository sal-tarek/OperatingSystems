#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RoundRobin.h"

// runs one cycle of the RR scheduler
void runRR(int quantum)
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

        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        printf("Executing %d\n", runningProcess->pid);

        if(runningProcess->state == BLOCKED)
        {
            if (runningProcess->quantumUsed == quantum)
            {
                printf("moved %d to level 0\n", runningProcess->pid);
            }

            runningProcess->quantumUsed = 0;
            runningProcess = NULL;
        }
        else{
            if (runningProcess->remainingTime == 0)
            {
                dequeue(readyQueues[0]); // Now we safely remove it from the queue
                printf("Finished %d\n", runningProcess->pid);
                setProcessState(runningProcess->pid, TERMINATED);
            }
            else if (runningProcess->quantumUsed == quantum)
            {
                dequeue(readyQueues[0]);
                enqueue(readyQueues[0], runningProcess);
                printf("moved %d to level 0\n", runningProcess->pid);

                runningProcess->quantumUsed = 0;
                runningProcess = NULL;
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