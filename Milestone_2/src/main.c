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

#define numProcesses 3
#define numQueues 4

// Global variables
Queue *readyQueues[numQueues]; // Ready Queue holding processes waiting to run
Process *runningProcess = NULL; // Currently running process (or NULL if none)
int clockCycle; // Current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *blocked_queue = NULL;

// Timeout callback to advance simulation
static gboolean on_timeout(gpointer user_data) {
    if (getProcessState(1) != TERMINATED || 
        getProcessState(2) != TERMINATED || 
        getProcessState(3) != TERMINATED) {
        populateMemory();
        controller_update_all(); // Update UI
        runMLFQ();
        clockCycle++;
        return G_SOURCE_CONTINUE;
    }
    printf("Simulation completed.\n");
    return G_SOURCE_REMOVE;
}

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
    for (int i = 0; i < numQueues; i++) 
        readyQueues[i] = createQueue();

    // Create blocked_queue
    blocked_queue = createQueue();
    if (!blocked_queue) {
        fprintf(stderr, "Failed to create blocked_queue\n");
        freeQueue(job_pool);
        for (int i = 0; i < numQueues; i++)
            freeQueue(readyQueues[i]);
        return 1;
    }

    // Create processes
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 3);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(job_pool);
        for (int i = 0; i < numQueues; i++)
            freeQueue(readyQueues[i]);
        freeQueue(blocked_queue);
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
    for (int i = 0; i < numQueues; i++)
        freeQueue(readyQueues[i]);
    freeQueue(blocked_queue);

    return status;
}