#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "Queue.h"
#include "memory.h"
#include "MLFQ.h"
#include "RoundRobin.h"

#define numProcesses 3
#define numQueues 4

Queue *readyQueues[numQueues];              // Ready Queue holding processes waiting to run by the chosen Scheduler
Process *runningProcess = NULL;             // currently running process (or NULL if none)
int clockCycle;                             // current clock cycle of the simulation

int main() {
    clockCycle = 0;

    // Initialize memory hashmap (empty for now)
    MemoryWord *memory = NULL; // Will store address-to-data mappings

    // Create job_pool queue
    Queue *job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create ready queues
    for (int i = 0; i < numQueues; i++) 
        readyQueues[i] = createQueue();

    // Create 3 Process structs
    Process *p1 = createProcess(1, "p1.txt", 0, 7);  // P1: arrival 0, burst 7
    Process *p2 = createProcess(2, "p2.txt", 2, 7);  // P2: arrival 2, burst 7
    Process *p3 = createProcess(3, "p3.txt", 5, 10); // P3: arrival 5, burst 10

    // Enqueue processes in job_pool
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    display(job_pool);
    

    // Trying Schedulers
    // Uncomment the scheduler you want to test


    //  MLFQ

    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P1 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P2 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P3 to readyQueue
    // while(p1->state != TERMINATED || p2->state != TERMINATED || p3->state != TERMINATED) {
    //     runMLFQ(); 
    //     clockCycle++; 
    // }



    // RR

    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P1 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P2 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P3 to readyQueue
    // // Get quantum from user
    // int q;
    // do {
    //     printf("Enter quantum (positive integer): ");
    //     if (scanf("%d", &q) != 1) { // Invalid input: not a number
    //         printf("Invalid input. Please enter a number.\n");
    //         q = -1; 
    //     } else if (q <= 0) {
    //         printf("Quantum must be a positive integer.\n");
    //     }
        
    //     // clear the input buffer after using scanf
    //     int c;
    //     while ((c = getchar()) != '\n' && c != EOF); 
    // } while (q <= 0);
        
    // while(p1->state != TERMINATED || p2->state != TERMINATED || p3->state != TERMINATED) {
    //     runRR(q); 
    //     clockCycle++; 
    // }



    // FCFS
    
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P1 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P2 to readyQueue
    // enqueue(readyQueues[0], dequeue(job_pool)); // Move P3 to readyQueue
    // while(p1->state != TERMINATED || p2->state != TERMINATED || p3->state != TERMINATED) {
    //     runFCFS(); 
    //     clockCycle++; 
    // }


    // Cleanup (frees Process structs and file_path)
    freeQueue(job_pool);

    return 0;
}


