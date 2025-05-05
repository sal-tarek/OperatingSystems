#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FCFS.h"
#include "parser.h"

// runs one cycle of the FCFS scheduler
void runFCFS()
{

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

    // If a process is running, execute it
    if (runningProcess != NULL)
    {
        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING;

        exec_cycle(runningProcess); // Execution of the next instruction of the process

        runningProcess->remainingTime--;
        printf("Executed Process %d remaining time: %d time in queue: %d\n", runningProcess->pid, runningProcess->remainingTime, runningProcess->timeInQueue);

        // Check if the process has finished
        if (runningProcess->remainingTime == 0)
        {
            dequeue(readyQueues[0]); // Remove the process from the queue
            setProcessState(runningProcess->pid, TERMINATED);
            printf("Process %d finished execution at time %d\n", runningProcess->pid, clockCycle);
            runningProcess = NULL; // Clear runningProcess
        }
        else
        {
            setProcessState(runningProcess->pid, READY);
            runningProcess->state = READY;
        }
    }
    else
    {
        printf("CPU is idle\n");
    }

    // Remove the blokced process from the ready queues because they've already been put in the blocked Queue, so they don't appear in the ready queues the next cycle
    int size = getQueueSize(readyQueues[0]);
    for (int j = 0; j < size; j++)
    {
        Process *curr = dequeue(readyQueues[0]);
        if (curr->state != BLOCKED)
            enqueueWithoutClone(readyQueues[0], curr);
    }

    // Print ready queue
    printf("Ready ");
    displayQueueSimplified(readyQueues[0]);
}