#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"
#include "process.h"

void runMLFQ(Process* processes[], int numProcesses) {
    printf("MLFQ Scheduler Simulation\n");

    Queue* queues[4];  
    for (int i = 0; i < 4; i++) 
        queues[i] = createQueue();
    
    int finished = 0;   // Number of finished processes
    int time = 0;       // Current Cycle of the CPU (incremented at the end of each cycle)


    while(finished < numProcesses){
        printf("\nTime %d: \n \n", time);

        // Add newly arrived processes to highest priority queue
        for (int i = 0; i < numProcesses; i++) {        // what happens if 2/more arrived at the same time ?????????
            if (processes[i]->ready_time == time) {
                // current->priority = 0;
                enqueue(queues[0], processes[i]);
                printf("Process %d arrived\n", processes[i]->pid);
            }
        }

        // Display the ready queues
        display(queues[0]);
        display(queues[1]);
        display(queues[2]);
        display(queues[3]);

        Process* current = NULL;
        int level = -1;

        // Find the highest priority existing process in the queues
        for (int i = 0; i < 4; i++) {
            if (!isEmpty(queues[i])) {
                current = dequeue(queues[i]);
                level = i;
                break;
            }
        }

        if(current != NULL){
            int timeQuantum = 1 << level; // time quantum = 2^i for queue i {Queue 0 --> 1, Queue 1 --> 2, Queue 2 --> 4, Queue 3 --> 8}
            current->state = RUNNING;

            // Execute Process for the time quantum
            for (int j = 0; j < timeQuantum && current->remainingTime > 0; j++) {

                if(j == 0)
                    printf("Running process %d in queue %d with time quantum %d\n", current->pid, level+1, timeQuantum);

                // Execute(current->pid); // Simulate the execution of the process
                current->remainingTime--;
                printf("Executing next instruction of process %d\n", current->pid);

                if(current->remainingTime > 0 && j+1 < timeQuantum){ // Not the final cycle
                    time++; 
                    printf("\nTime %d:\n \n", time);
                }
            }
            
            if (current->remainingTime == 0) {
                printf("Process %d finished\n", current->pid);
                finished++;
                current->state = TERMINATED;
            } else {
                if(level != 3) {
                    // current->priority = level+1;
                    enqueue(queues[level+1], current); // Move to next queue
                    printf("Process %d is moved to level %d\n", current->pid, level+2);
                }
                else{
                    enqueue(queues[level], current); // Stay in the same queue (Last Queue - RR)
                    printf("Process %d is moved to level %d\n", current->pid, level+1);
                }
                current->state = READY;
            }
            time++;
        }else{
            printf("CPU is idle\n", time++);
        }

    }
    return;
}

int main() {
    // Test sample 

    int n = 4;
    Process* processes[n];

    int ready[] = {0, 2, 2, 3};
    int burst[] = {1, 3, 8, 15};

    for (int i = 0; i < n; i++) {
        processes[i] = (Process*)malloc(sizeof(Process));
        processes[i]->pid = i + 1;
        processes[i]->ready_time = ready[i];
        processes[i]->burstTime = burst[i];
        processes[i]->remainingTime = burst[i];
        processes[i]->next = NULL;
    }

    runMLFQ(processes, n);
    return 0;
}