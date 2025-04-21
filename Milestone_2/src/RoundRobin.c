#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "queue.h"

extern Queue *readyQueue;
extern Process *runningProcess; // The currently running process (or NULL if none)
extern int clockCycle; // Tracks the current clock cycle of the simulation


int quantum;                                    // Stores the user-defined quantum (time slice) for RR
Process* rrProcesses[20];                    // Array to store all RRProcess instances for tracking
int rrProcessCount = 0;                         // Counts how many RRProcess instances we’ve created

// runs one cycle of the RR scheduler
void roundRobinScheduler() {                    
    clockCycle++;


    if (runningProcess) {                       // Checks if there’s a process currently running
        runningProcess->quantumUsed++;  
        runningProcess->remainingTime--;      

        // Execute(current->pid);           // Simulate the execution of the process

        if (runningProcess->state == TERMINATED) {
            // Remove from rrProcesses to prevent re-enqueue
            for (int i = 0; i < rrProcessCount; i++) {
                if (rrProcesses[i] == runningProcess) {
                    rrProcesses[i] = NULL; // Mark as freed
                    break;
                }
            }
            freeRRProcess(runningProcess);
            runningProcess = NULL;
        } else if (runningProcess->quantumUsed == quantum) { 
            runningProcess->state = READY; 
            runningProcess->quantumUsed = 0;    
            enqueue(readyQueue, runningProcess); 
            runningProcess = NULL;   
            
            if(!isEmpty(readyQueue)) {                 
                runningProcess = dequeue(readyQueue); 
                runningProcess->state = RUNNING; 
            }
        }
    }

    printf("Clock Cycle %d: ", clockCycle);     
    if (runningProcess) {                       
        printf("Running PID %d\n", runningProcess->pid);
    } else {                                    
        printf("No running process\n");         
    }

    // printing the current Ready Queue
    printf("Ready ");                          
    display(readyQueue);                        
}

int main() {                                    
    initRoundRobin();     

    // Get quantum from user
    int q;
    do {
        printf("Enter quantum (positive integer): ");
        if (scanf("%d", &q) != 1) { // Invalid input: not a number
            printf("Invalid input. Please enter a number.\n");
            q = -1; 
        } else if (q <= 0) {
            printf("Quantum must be a positive integer.\n");
        }
        
        // clear the input buffer after using scanf
        int c;
        while ((c = getchar()) != '\n' && c != EOF); 
    } while (q <= 0);
    quantum = q; 


    // Create test processes
    Process* p1 = createProcess(1, "p1.txt", 0, 5); 
    Process* p2 = createProcess(2, "p2.txt", 0, 3); 
    Process* p3 = createProcess(3, "p3.txt", 0, 4); 

    // Enqueue processes
    enqueue(readyQueue, p1);           
    enqueue(readyQueue, p2);           
    enqueue(readyQueue, p3);           

    // Run scheduler
    while (!isEmpty(readyQueue) || runningProcess) { 
        printf("\nPress Enter to advance to next cycle..."); 
        getchar();                                              // Waits for Enter key
        roundRobinScheduler();                                  // Runs one cycle of the scheduler
    }

    // Clean up
    freeQueue(readyQueue);                      
    for (int i = 0; i < rrProcessCount; i++) {  
        if (rrProcesses[i]) {
            freeRRProcess(rrProcesses[i]);      
        }
    }
    return 0;
}