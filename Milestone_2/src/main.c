// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "Queue.h"
#include "memory.h"

int main() {
    // Initialize memory hashmap (empty for now)
    MemoryWord *memory = NULL; // Will store address-to-data mappings

    // Create job_pool queue
    Queue *job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create 3 Process structs
    Process *p1 = createProcess(1, "p1.txt", 0, 7);  // P1: arrival 0, burst 7
    Process *p2 = createProcess(2, "p2.txt", 2, 7);  // P2: arrival 2, burst 7
    Process *p3 = createProcess(3, "p3.txt", 5, 10); // P3: arrival 5, burst 10

    // Enqueue processes in job_pool
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);

    // Display job_pool (for verification)
    display(job_pool);

    // Cleanup
    // freeQueue frees Process structs and file_path
    freeQueue(job_pool);

    return 0;
}