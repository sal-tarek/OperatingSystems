// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "Queue.h"
#include "memory.h"
#include "index.h"
#include "memory_manager.h"


int main() {
    // Initialize memory hashmap (empty for now)
    MemoryWord *memory = NULL; // Will store address-to-data mappings
    IndexEntry *index = NULL; // Will store instruction-to-address mappings

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
    enqueueSortedByArrivalTime(job_pool, p1);
    enqueueSortedByArrivalTime(job_pool, p2);
    enqueueSortedByArrivalTime(job_pool, p3);

    // Display job_pool (for verification)
    displayQueue(job_pool);

//simulating a clock and populating accordingly
//     int i =0;
//     while (!isQueueEmpty(job_pool)) {
//         printf("time stamp:%d\n",i);
//        //call populateMemory to read instructions and populate PCB
//        populateMemory(job_pool, &memory, &index, i);
//        i++;
//     //printf("Memory after populating:\n");
//     printMemory(memory);
//   }

   // trying to populate all the processes in the job_pool
    //   populateMemory(job_pool, &memory, &index, 5);
    //   printMemory(memory);

    
       // how to fecth for PCB elements
    //         //PID:1
    //         // Address: 10 => Data: State:NEW
    //         // Address: 11 => Data: Priority:0
    //         // Address: 12 => Data: PC:0
    //         // Address: 13 => Data: MemLower:0
    //         // Address: 14 => Data: MemUpper:14


    // //fetch data by index
    // char *data = fetchDataByIndex(index, memory, "P1_PCB_4");
    // if (data) {
    //     printf("Fetched data for P1_Instruction_1: %s\n", data);
    // } else {
    //     printf("Data not found for P1_Instruction_1\n");
    // }

    // // Update data by index
    // int result = updateDataByIndex(index, memory, "P1_Variable_1", "42"); // User input
    // if (result == 0) {
    //     printf("Updated data for P1_Variable_1 successfully\n");
    //     printMemory(memory);
    // } else {
    //     printf("Failed to update data for P1_Variable_1\n");
    // }
    

    // Cleanup
    // freeQueue frees Process structs and file_path
    //freeQueue(job_pool);

    return 0;
}