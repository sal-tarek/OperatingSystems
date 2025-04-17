#ifndef PCB_H
#define PCB_H

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

Process* createProcess(int id);

#endif