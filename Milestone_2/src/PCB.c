#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;


typedef struct Process {
    int id;                   
    ProcessState state;       
    int priority;             
    int programCounter;       
    int memLowerBound;        
    int memUpperBound;        

    // (used for scheduling)
    int arrivalTime;          
    int burstTime;            // Total execution time required
    int remainingTime;        

    struct Process* next;     // For linking in queues (It's like the next pointer in Node Struct)

} Process;


Process* createProcess(int id) {
    Process* newProcess = (Process*)malloc(sizeof(Process));
    if (!newProcess) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    newProcess->id = id;
    newProcess->state = NEW;
    newProcess->priority = 0;
    newProcess->programCounter = 0;
    newProcess->memLowerBound = 0;
    newProcess->memUpperBound = 0;
    newProcess->arrivalTime = 0;
    newProcess->burstTime = 0;
    newProcess->remainingTime = 0;
    newProcess->next = NULL;

    return newProcess;
}