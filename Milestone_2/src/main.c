#include <stdio.h>
#include <stdlib.h>
#include "memory_manager.h"
#include "process.h"
#include "Queue.h"
#include "memory.h"
#include "MLFQ.h"
#include "RoundRobin.h"
#include "FCFS.h"
#include "index.h"
#include "PCB.h"
#include "mutex.h"

#define numProcesses 3
#define numQueues 4
<<<<<<< HEAD


=======
//gcc FCFS.c index.c instruction.c main.c memory_manager.c memory.c MLFQ.c mutex.c parser.c PCB.c process.c Queue.c RoundRobin.c -o main
>>>>>>> Memory
// Global variables
Queue *readyQueues[numQueues];              // Ready Queue holding processes waiting to run by the chosen Scheduler
Process *runningProcess = NULL;             // currently running process (or NULL if none)
int clockCycle = 0;                             // current clock cycle of the simulation
//Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numberOfProcesses = 0;
char* schedulingAlgorithm = NULL;
//Queue *processes = NULL; // Blocked queue for processes waiting for I/O

int main2() {
    clockCycle = 0;

    // Initialize memory hashmap (empty for now)
    MemoryWord *memory = NULL; // Will store address-to-data mappings

<<<<<<< HEAD
    // // Create job_pool queue
    // job_pool = createQueue();
    // if (!job_pool) {
    //     fprintf(stderr, "Failed to create job_pool\n");
    //     return 1;
=======
    // Create job_pool queue
    job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create ready queues
    for (int i = 0; i < numQueues; i++) 
        readyQueues[i] = createQueue();

    // Create global blocked queue
    global_blocked_queue = createQueue();

    // Create blocked_queue
    /*
    blocked_queue = createQueue();
    if (!blocked_queue) {
        fprintf(stderr, "Failed to create blocked_queue\n");
        freeQueue(job_pool);
        for(int i = 0; i < 4; i++)
            freeQueue(readyQueues[i]);
        return 1;
    }*/

    // Create processes
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 1);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 2);
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(job_pool);
        for(int i = 0; i < 4; i++)
            freeQueue(readyQueues[i]);
        return 1;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    printf("Job Pool ");
    displayQueue(job_pool);
    printf("\n");

    //populate memory test
    while(clockCycle < 3) {
        printf("Time %d\n", clockCycle);
        populateMemory();
        printMemory();
        clockCycle++;
        }
    deleteProcessFromMemory(1);
    printMemory();
   

    // //MLFQ
    // while(getProcessState(1) != TERMINATED|| getProcessState(2) != TERMINATED|| getProcessState(3) != TERMINATED) {
    //     populateMemory();
    //     //printMemory();
    //     runMLFQ(); 
    //     if (clockCycle++ == 30) break;
>>>>>>> Memory
    // }

    // // Create ready queues
    // for (int i = 0; i < numQueues; i++) 
    //     readyQueues[i] = createQueue();

    // // Create global blocked queue
    // global_blocked_queue = createQueue();

    // // Create blocked_queue
    // /*
    // blocked_queue = createQueue();
    // if (!blocked_queue) {
    //     fprintf(stderr, "Failed to create blocked_queue\n");
    //     freeQueue(job_pool);
    //     for(int i = 0; i < 4; i++)
    //         freeQueue(readyQueues[i]);
    //     return 1;
    // }*/

    // // Create processes
    // Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    // Process *p2 = createProcess(2, "../programs/Program_2.txt", 3);
    // Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    // if (!p1 || !p2 || !p3) {
    //     fprintf(stderr, "Failed to create processes\n");
    //     freeQueue(job_pool);
    //     for(int i = 0; i < 4; i++)
    //         freeQueue(readyQueues[i]);
    //     return 1;
    // }

    // // Enqueue processes
    // enqueue(job_pool, p1);
    // enqueue(job_pool, p2);
    // enqueue(job_pool, p3);
    // printf("Job Pool ");
    // displayQueue(job_pool);
    // printf("\n");


    // // Schedulers

    // // while(clockCycle<5){
    // //     populateMemory();
    // //     clockCycle++;
    // // }

    // // MLFQ
    // // while((getProcessState(1) != TERMINATED|| getProcessState(2) != TERMINATED|| getProcessState(3) != TERMINATED) && clockCycle<6) {
    // //     populateMemory();
    // //     printMemory();
    // //     runMLFQ(); 
    // //     clockCycle++;
    // // }

    // // RR
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
    // while(getProcessState(1) != TERMINATED|| getProcessState(2) != TERMINATED|| getProcessState(3) != TERMINATED) {
    //     populateMemory();
    //     printMemory();
    //     runRR(q); 
    //     clockCycle++; 
    // }

<<<<<<< HEAD
    // // // FCFS
    // // while(getProcessState(1) != TERMINATED || getProcessState(2) != TERMINATED || getProcessState(3) != TERMINATED) {
    // //     populateMemory();
    // //     displayProcess(runningProcess);
    // //     runFCFS(); 
    // //     clockCycle++; 
    // // }


    // // Test 1: Populate memory at time 0
    // printf("Populating memory at time 0...\n");
    // printf("size: %d\n", getQueueSize(job_pool));
    // // populateMemory();
    // printMemory();
    // // displayMemoryRange(0); // Show all memory ranges
=======
    //FCFS
    // while(getProcessState(1) != TERMINATED || getProcessState(2) != TERMINATED || getProcessState(3) != TERMINATED) {
    //     populateMemory();
    //     displayProcess(runningProcess);
    //     runFCFS(); 
    //     clockCycle++; 
    // }


    // Test 1: Populate memory at time 0
    
    // printf("Populating memory at time 0...\n");
    // printf("size: %d\n", getQueueSize(job_pool));
    // populateMemory();
    // printMemory();

    // displayMemoryRange(0); // Show all memory ranges
>>>>>>> Memory


    // // Test 2: Fetch instruction (P1_Instruction_1)
   
    // // DataType type;
    // // void *data = fetchDataByIndex("P1_Instruction_1", &type);
    // // if (data && type == TYPE_STRING) {
    // //     printf("Fetched P1_Instruction_1: %s\n", (char*)data);
    // // } else {
    // //     printf("Failed to fetch P1_Instruction_1\n");
    // // }

    // // DataType type;
    // // // Test 3: Fetch variable (P1_Variable_1)
    // // void *data = fetchDataByIndex("P1_Variable_c", &type);
    // // if (data && type == TYPE_STRING) {
    // //     printf("Fetched P1_Variable_c: %s\n", (char*)data);
    // // } else {
    // //     printf("Failed to fetch P1_Variable_c\n");
    // // }

    // // // Test 4: Fetch PCB (P1_PCB)
    // // data = fetchDataByIndex("P1_PCB", &type);
    // // if (data && type == TYPE_PCB) {
    // //     struct PCB *pcb = (struct PCB*)data;
    // //     printf("Fetched P1_PCB: PID=%d, State=%d, PC=%d, MemLower=%d, MemUpper=%d\n",
    // //            getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb),
    // //            getPCBMemLowerBound(pcb), getPCBMemUpperBound(pcb));
    // // } else {
    // //     printf("Failed to fetch P1_PCB\n");
    // // }

    // // // Test 6: Update PCB fields directly
    // // printf("Updating P1_PCB state to RUNNING and PC to 2...\n");
    // // data = fetchDataByIndex("P1_PCB", &type);
    // // if (data && type == TYPE_PCB) {
    // //     struct PCB *pcb = (struct PCB*)data;
    // //     setPCBState(pcb, RUNNING);
    // //     setPCBProgramCounter(pcb, 2);
    // //     printf("Updated P1_PCB: PID=%d, State=%d, PC=%d\n",
    // //            getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb));
    // // }

    // // // Test 7: Replace PCB via updateDataByIndex
    // // printf("Replacing P1_PCB with new PCB...\n");
    // // struct PCB *new_pcb = createPCBWithBounds(1, 0, 14);
    // // new_pcb->state = TERMINATED;
    // // new_pcb->programCounter = 3;
    // // if (updateDataByIndex("P1_PCB", new_pcb, TYPE_PCB) == 0) {
    // //     data = fetchDataByIndex("P1_PCB", &type);
    // //     if (data && type == TYPE_PCB) {
    // //         struct PCB *pcb = (struct PCB*)data;
    // //         printf("New P1_PCB: PID=%d, State=%d, PC=%d\n",
    // //                getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb));
    // //     }
    // // } else {
    // //     printf("Failed to replace P1_PCB\n");
    // //     freePCB(new_pcb);
    // // }

    // // Cleanup
    // freeMemoryWord();
    // freeIndex(&index_table);
    // freeQueue(job_pool);
    // for(int i = 0; i < 4; i++)
    //     freeQueue(readyQueues[i]);

    // printf("Test completed.\n");
    return 0;
}
