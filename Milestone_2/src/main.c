#include <stdio.h>
#include <stdlib.h>
#include "memory_manager.h"
#include "process.h"
#include "Queue.h"
#include "memory.h"
#include "index.h"
#include "PCB.h"
#include "mutex.h"
int main() {
    // Initialize job_pool
    Queue *job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create processes
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0, 10);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 2, 8);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 4, 12);
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(job_pool);
        // freeProcess(p1);
        // freeProcess(p2);
        // freeProcess(p3);
        return 1;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);

    // Initialize memory and index
    MemoryWord *memory = NULL;
    IndexEntry *index = NULL;

    // Test 1: Populate memory at time 0
    printf("Populating memory at time 0...\n");
    populateMemory(job_pool, &memory, &index, 0);
    printMemory(memory);
    displayMemoryRange(0); // Show all memory ranges

    // Test 2: Fetch instruction (P1_Instruction_1)
    DataType type;
    void *data = fetchDataByIndex(index, memory, "P1_Instruction_1", &type);
    if (data && type == TYPE_STRING) {
        printf("Fetched P1_Instruction_1: %s\n", (char*)data);
    } else {
        printf("Failed to fetch P1_Instruction_1\n");
    }

    // Test 3: Fetch variable (P1_Variable_1)
    data = fetchDataByIndex(index, memory, "P1_Variable_1", &type);
    if (data && type == TYPE_STRING) {
        printf("Fetched P1_Variable_1: %s\n", (char*)data);
    } else {
        printf("Failed to fetch P1_Variable_1\n");
    }

    // Test 4: Fetch PCB (P1_PCB)
    data = fetchDataByIndex(index, memory, "P1_PCB", &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        printf("Fetched P1_PCB: PID=%d, State=%d, PC=%d, MemLower=%d, MemUpper=%d\n",
               getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb),
               getPCBMemLowerBound(pcb), getPCBMemUpperBound(pcb));
    } else {
        printf("Failed to fetch P1_PCB\n");
    }

    // // Test 5: Update variable (P1_Variable_1)
   

    // Test 6: Update PCB fields directly
    printf("Updating P1_PCB state to RUNNING and PC to 2...\n");
    data = fetchDataByIndex(index, memory, "P1_PCB", &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        setPCBState(pcb, RUNNING);
        setPCBProgramCounter(pcb, 2);
        printf("Updated P1_PCB: PID=%d, State=%d, PC=%d\n",
               getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb));
    }

    // Test 7: Replace PCB via updateDataByIndex
    printf("Replacing P1_PCB with new PCB...\n");
     PCB *new_pcb = createPCBWithBounds(1, 0, 14);
    new_pcb->state = TERMINATED;
    new_pcb->programCounter = 3;
    if (updateDataByIndex(index, memory, "P1_PCB", new_pcb, TYPE_PCB) == 0) {
        data = fetchDataByIndex(index, memory, "P1_PCB", &type);
        if (data && type == TYPE_PCB) {
             PCB *pcb = ( PCB*)data;
            printf("New P1_PCB: PID=%d, State=%d, PC=%d\n",
                   getPCBId(pcb), getPCBState(pcb), getPCBProgramCounter(pcb));
        }
    } else {
        printf("Failed to replace P1_PCB\n");
    }

    // Test 8: Populate memory at time 4 (P2, P3)
    printf("Populating memory at time 4...\n");
    populateMemory(job_pool, &memory, &index, 4);
    printMemory(memory); 
    data = fetchDataByIndex(index, memory, "P3_PCB", &type);
    if (data && type == TYPE_PCB) {
        struct PCB *pcb = (struct PCB*)data;
        printf("Fetched P3_PCB: PID=%d, State=%d\n", getPCBId(pcb), getPCBState(pcb));
    }

    // // Cleanup
    // freeMemoryWord(memory);
    // freeIndex(index);
    // freeQueue(job_pool);

    printf("Test completed.\n");
    return 0;
}