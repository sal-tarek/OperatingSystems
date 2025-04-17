#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "PCB.h"

void runMLFQ(Process* processes[], int numProcesses) {
    printf("MLFQ Scheduler Simulation\n");

    Queue* queues[4];  
    for (int i = 0; i < 4; i++) 
        queues[i] = createQueue();
    
    int finished = 0;   // Number of finished processes
    int time = 0;       // Current Cycle of the CPU

    while(finished < numProcesses){

        // Add newly arrived processes to highest priority queue
        for (int i = 0; i < numProcesses; i++) { // what happens if 2/more arrived at the same time ?????????
            if (processes[i]->arrivalTime == time) {
                processes[i]->priority = 0;
                processes[i]->state = READY;
                enqueue(queues[0], processes[i]);
                printf("Time %d: Process %d arrived\n", time, processes[i]->id);
            }
        }

        // Display the queues
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
                    printf("Time %d: Running process %d in queue %d with time quantum %d\n", time+1, current->id, level+1, timeQuantum);

                current->programCounter++;
                // execute the instruction that has this program counter
                current->remainingTime--;
                time++; // Update the current cycle of the CPU
                
                printf("Time %d: Executing instruction %d of process %d\n", time, current->programCounter, current->id);
            }
            
            if (current->remainingTime == 0) {
                printf("Process %d finished\n", current->id);
                finished++;
            } else {
                if(level != 3) {
                    current->priority = level+1;
                    enqueue(queues[level+1], current); // Move to next queue
                    printf("Time %d: Time slice ended, demoting process %d to level %d\n", time, current->id, level+2);
                }
                else{
                    enqueue(queues[level], current); // Stay in the same queue (Last Queue - RR)
                    printf("Time %d: Time slice ended, demoting process %d to level %d\n", time, current->id, level+1);
                }
            }
        }else{
            printf("Time %d: CPU is idle\n", time++);
        }

    }
    return;
}

int main() {
    // Test sample 

    int n = 4;
    Process* processes[n];

    int arrival[] = {0, 2, 2, 3};
    int burst[] = {1, 3, 8, 15};

    for (int i = 0; i < n; i++) {
        processes[i] = (Process*)malloc(sizeof(Process));
        processes[i]->id = i + 1;
        processes[i]->arrivalTime = arrival[i];
        processes[i]->burstTime = burst[i];
        processes[i]->remainingTime = burst[i];
        processes[i]->next = NULL;
    }

    runMLFQ(processes, n);
    return 0;
}