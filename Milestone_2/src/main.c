#include <gtk/gtk.h>
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
#include "controller.h"
// #include "controller.h"
#include "console_view.h"
#include "console_controller.h"
#include "console_model.h"

#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support
#define MAX_NUM_QUEUES 4        // Maximum number of queues
#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support
#define MAX_NUM_QUEUES 4        // Maximum number of queues

// Global variables
Queue *readyQueues[MAX_NUM_QUEUES]; // Ready Queue holding processes waiting to run
Process *runningProcess = NULL; // Currently running process (or NULL if none)
int clockCycle; // Current clock cycle of the simulation
Queue *readyQueues[MAX_NUM_QUEUES]; // Ready Queue holding processes waiting to run
Process *runningProcess = NULL; // Currently running process (or NULL if none)
int clockCycle; // Current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numProcesses = 0; // Number of processes in the simulation

int main(int argc, char *argv[]) {
    clockCycle = 0;

    // Initialize memory
    memory = NULL; 
    // Create job_pool queue
    job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create ready queues
    for (int i = 0; i < MAX_NUM_QUEUES; i++) 
        readyQueues[i] = createQueue();

    // Create global blocked queue
    global_blocked_queue = createQueue();

    // Create processes
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    numProcesses++;
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 3);
    numProcesses++;
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    numProcesses++;
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(job_pool);
        for (int i = 0; i < MAX_NUM_QUEUES; i++)
            freeQueue(readyQueues[i]);
        freeQueue(global_blocked_queue);
        return 1;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    printf("Job Pool ");
    displayQueue(job_pool);
    printf("\n");

    // Start the UI and simulation
    int status = controller_start(argc, argv);

    // Cleanup
    freeMemoryWord();
    freeIndex(&index_table);
    freeQueue(job_pool);
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
        freeQueue(readyQueues[i]);
    freeQueue(global_blocked_queue);

    return status;
}