#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "Queue.h"

void runFCFS(Queue *jobPool) {
    Queue *readyQueue = createQueue();
    int clock = 0;

    printf("=== FCFS Scheduling Simulation ===\n");


        // Print state of the ready queue
        printf("Ready Queue: ");
        display(readyQueue);
        printf("\n");

        // Run process at the front of the ready queue
        if (!isEmpty(readyQueue)) {
            Process *running = peek(readyQueue);

            if (running->state != RUNNING) {
                running->state = RUNNING;
                printf("Process %d started running at time %d\n", running->pid, clock);
            }

            running->remainingTime--;

            // Debugging remaining time
            printf("Process %d remaining time: %d\n", running->pid, running->remainingTime);

            if (running->remainingTime == 0) {
                running = dequeue(readyQueue);
                running->state = TERMINATED;
                printf("Process %d finished execution at time %d\n", running->pid, clock);
                free(running);
            }
        } 
        else {
            printf("CPU is IDLE at time %d\n", clock);
        }

        // Increment clock
        clock++;

    printf("=== FCFS Scheduling Completed ===\n");
}
