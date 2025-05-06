#include <stdio.h>
#include <stdlib.h>
#include "MLFQ.h"
#include "parser.h"

int lastUsedLevel = -1; // -1 means no pending process

// runs one cycle of the MLFQ scheduler
void runMLFQ()
{
    // Display the ready queues
    printf("Before ");
    for (int i = 0; i < 4; i++)
        displayQueueSimplified(readyQueues[i]);
    printf("Blocked ");
    displayQueueSimplified(global_blocked_queue);

    // there is no pending process(didn't complete its quantum), so we need to find the next process to execute
    if (lastUsedLevel == -1)
    {
        for (int i = 0; i < MAX_NUM_QUEUES; i++)
        {
            if (!isEmpty(readyQueues[i]))
            {
                runningProcess = peek(readyQueues[i]);
                lastUsedLevel = i;
                break;
            }
        }
    }

    // Update the timeInQueue for all processes in the ready queue
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        Process *temp = readyQueues[i]->front;
        while (temp != NULL)
        {
            temp->timeInQueue++;
            temp = temp->next;
        }
    }

    if (runningProcess != NULL)
    {
        int timeQuantum = 1 << lastUsedLevel; // time quantum = 2^i for queue i {Queue 0 --> 1, Queue 1 --> 2, Queue 2 --> 4, Queue 3 --> 8}

        setProcessState(runningProcess->pid, RUNNING);
        runningProcess->state = RUNNING;

        exec_cycle(runningProcess);

        runningProcess->quantumUsed++;
        runningProcess->remainingTime--;

        printf("Executed Process %d remaining time: %d time in queue: %d\n", runningProcess->pid, runningProcess->remainingTime, runningProcess->timeInQueue);

        if(runningProcess->state == BLOCKED)
        {
            if (runningProcess->remainingTime == 0)
            {
                setProcessState(runningProcess->pid, TERMINATED);
                runningProcess->state = TERMINATED;
    
                printf("Finished %d\n", runningProcess->pid);
            }
            else if (runningProcess->quantumUsed == timeQuantum)
            {
                if (lastUsedLevel != 3)
                {
                    setProcessPriority(runningProcess->pid, getProcessPriority(runningProcess->pid) + 1); // Increase priority
                    printf("moving %d to Level %d next clock cycle\n", runningProcess->pid, lastUsedLevel + 2);
                }
                else
                {
                    printf("moving %d to Level %d\n next clock cycle", runningProcess->pid, lastUsedLevel + 1);
                }
            }

            runningProcess->quantumUsed = 0;
            runningProcess = NULL;
            lastUsedLevel = -1;
        }
        else{
            if (runningProcess->remainingTime == 0)
            {
                dequeue(readyQueues[lastUsedLevel]); // Now we safely remove it from the queue
                printf("Finished %d\n", runningProcess->pid);
        
                setProcessState(runningProcess->pid, TERMINATED);
                runningProcess->state = TERMINATED;
                runningProcess->quantumUsed = 0;
                runningProcess = NULL;
                lastUsedLevel = -1;
            }
            else if (runningProcess->quantumUsed == timeQuantum)
            {
                dequeue(readyQueues[lastUsedLevel]);
                if (lastUsedLevel != 3)
                {
                    enqueue(readyQueues[lastUsedLevel + 1], runningProcess);                  // Move to next queue
                    setProcessPriority(runningProcess->pid, getProcessPriority(runningProcess->pid) + 1); // Increase priority
                    printf("moving %d to Level %d next clock cycle\n", runningProcess->pid, lastUsedLevel + 2);
                }
                else
                {
                    enqueue(readyQueues[lastUsedLevel], runningProcess); // Stay in the same queue (Last Queue - RR)
                    printf("moving %d to Level %d\n next clock cycle", runningProcess->pid, lastUsedLevel + 1);
                }

                runningProcess->quantumUsed = 0;
                setProcessState(runningProcess->pid, READY);
                runningProcess->state = READY;
                runningProcess = NULL;
                lastUsedLevel = -1;
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