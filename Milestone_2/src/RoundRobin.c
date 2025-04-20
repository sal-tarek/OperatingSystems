#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "PCB.h"
#include "Queue.h"

/*typedef struct PCB {
    int processID;
    int programCounter;
    int instructionCount;
    char** instructions;
    //Assumed other fields for PCB are declared by the person responsible for PCB.
    //int state;  0=RUNNING, 1=READY, 2=BLOCKED, 3=TERMINATED OR it can be enum depending on the person who implemented PCB.
    int timeInQueue; // Added for RR, it tracks the time (in clock cycles) the process has spent in the Ready Queue
    int quantumUsed; // Added for RR, it tracks the number of clock cycles the process has used in its current quantum
}PCB;

// Node for Ready Queue (linked list)
typedef struct QueueNode {
    PCB* pcb;                  //I'm storing a pointer to the PCB of the process in this node
    struct QueueNode* next;    // this points to the next node in the linked list
}QueueNode;

// This is the structure of the Ready Queue
typedef struct Queue{
    QueueNode* front;   //points to the first node in the queue
    QueueNode* rear;    //points to the last node in the queue
}Queue;

// These are my global variables
Queue readyQueue;           // Ready Queue for RR
PCB* runningProcess = NULL; // Currently running process
int clockCycle = 0;         // Current clock cycle
int quantum;                // User-defined quantum

// Initialize the scheduler
void initRoundRobin() { //I called it at the start of the simulation in order to reset the system
    readyQueue.front = NULL;    //this indicates an empty queue
    readyQueue.rear = NULL;
    runningProcess = NULL;      //indicates no process is running at the start
    clockCycle = 0;
}

// Enqueue a process to Ready Queue
void enqueue(Queue* q, PCB* pcb) {  //here I'm adding a PCB to the end of the ready queue
    Process* node = (Process*)malloc(sizeof(Process));        //allocating a memory for a new queue node
    node->pcb = pcb;        //setting the node's PCB pointer to the inputted PCB
    node->next = NULL;      //setting the node's next pointer to NULL indicating that this node will be the last in the queue until another one is added
    if (q->rear == NULL) {  //here I'm checking if the queue is empty
        q->front = q->rear = node;      //if the rear is NULL then the queue has no nodes so the new node becomes both the front and rear
    } else {
        q->rear->next = node;       //setting the "next" pointer of the last node to the new node
        q->rear = node;             //updating the rear poimter to the new node, making the new node ready for the next enqueue
    }
    pcb->timeInQueue = 0;           //resetting the process's time in the ready queue
    pcb->state = READY; // setting the state to READY
}

// Dequeue a process from Ready Queue
PCB* dequeue(Queue* q) {        //here I'm removing and returning the PCB at the front of the ready queue
    if (q->front == NULL) return NULL;  //if the front is NULL thus no processes to dequeue so return NULL
    Process* temp = q->front;     //storing the front node in a temp pointer, saving the node to be removed so that I can access its PCB later if needed
    PCB* pcb = temp->pcb;           //retrieving the PCB from the front node
    q->front = q->front->next;      //updating the queue by removing the front node making the next node (if available) the new front
    if (q->front == NULL) q->rear = NULL;   //if front is NULL after dequeuing then the queue is empty thus set rear to NULL
    free(temp);     //here I'm freeing memory of the dequeued node
    return pcb;
}

// Set quantum from user input
void setQuantum(int q) {
    if (q > 0) {
        quantum = q;        //setting the global quantum to the user imput
    } else {
        quantum = 3; //I thought of doing a default quantum in case the user entered an invalid input
    }
}

PCB* createMockPCB(int id, int instructionCount) {  //creating a PCB with id and instructionCount for my testing
    PCB* pcb = (PCB*)malloc(sizeof(PCB));
    pcb->processID = id;
    pcb->state = 1; //setting state to Ready
    pcb->programCounter = 0;
    pcb->instructionCount = instructionCount;
    pcb->timeInQueue = 0;
    pcb->quantumUsed = 0;
    pcb->instructions = (char**)malloc(instructionCount * sizeof(char*));
    for (int i = 0; i < instructionCount; i++) {    //each instr. is a dummy string as the actual ones are implemented by the one responsible for PCB
        pcb->instructions[i] = strdup("mock_instruction");
    }
    return pcb;
}

void freePCB(PCB* pcb){    //this shouls be called when the process terminates
    for (int i = 0; i < pcb->instructionCount; i++) {
        free(pcb->instructions[i]); //freeing each instr. string
    }
    free(pcb->instructions);    //freeing the instructions array
    free(pcb);  //freeing the PCB itself
}

// Assuming this function is implemented by the person responsible for the program syntax
// Returns: 0=blocked, 1=executed, 2=terminated
int executeInstruction(PCB* pcb){   //it should execute one instr. and update the program counter and return the status: 0: Blocked, 1:Executed successfully, 2:Terminated
    if (pcb->programCounter >= pcb->instructionCount) {     //checks if all instructions are done and returns 2 (terminated) if true
        return 2; //Terminated
    }
    pcb->programCounter++;  //otherwise increments pc and returns 1 (executed)
    return 1; //Executed
}

void roundRobinScheduler(){
    clockCycle++;

    // Update time in Ready Queue
    Process* current = readyQueue.front;      //this is a pointer to traverse the ready queue, starts at the front of the queue to update the timeInQueue for all ready processes
    while (current) {       //loops through all nodes in the ready qeue
        current->pcb->timeInQueue++;        //increments the waiting time for a process in the ready queue by accessing the pcb field then the timeInQueue field
        current = current->next;    //moves to the next node in the queue
    }

    // Handle running process
    if (runningProcess) {       //checks if a process is currently running, if runningProcess is not NULL then I'll execute its next instr. and check its status
        runningProcess->quantumUsed++;      //incrementing the cycles used by the running process in its current quantum in order to track how long the process has run to compare against "quantum" for preemption
        int status = executeInstruction(runningProcess);    //executes 1 instr of the running process

        if (status == 2) { // checks if process has terminated, Status 2 indicates all instructions are executed
            runningProcess->state = TERMINATED;  //Sets the process’s state to Terminated (marking the process as complete)
            runningProcess = NULL;  //indicates no process is running, allowing the scheduler to select a new process
        } else if (status == 0) { // Blocked
            runningProcess->state = WAITING;  //Sets the process’s state to Blocked.
            runningProcess = NULL; // allowing the scheduler to move to the next process, handled by mutex person
        } else if (runningProcess->quantumUsed >= quantum) { // checks if process's quantum expired
            runningProcess->state = READY;  //setting the process state to Ready, preparing it to be requeued
            runningProcess->quantumUsed = 0;    //Resets the process’s quantum usage
            enqueue(&readyQueue, runningProcess);   //Requeues the preempted process, by adding the PCB to the end of the Ready Queue
            runningProcess = NULL;  //Clears the running process pointer
        }
    }

    // Schedule next process
    if (!runningProcess && readyQueue.front) {  //this means if no process is running and the ready queue is not empty, select the next process
        runningProcess = dequeue(&readyQueue);  //dequeues the next PCB from the ready queue, setting the front process as the new running process
        runningProcess->state = 0; // setting the process state to RUNNING
        runningProcess->quantumUsed = 0;    //Resets the process’s quantum usage, starting the new process’s quantum at 0 allowing it to run for the full quantum
    }

    // Output for GUI integration
    printf("Clock Cycle %d: ", clockCycle); //current clock cycle to track the simulation progress
    if (runningProcess) {   //checking if a process is running to know whether to display running process details or write none
        printf("Running PID %d, PC %d\n", runningProcess->pid, runningProcess->programCounter);
    } else {
        printf("No running process\n");
    }
    printf("Ready Queue: ");
    current = readyQueue.front; //Resetting the traversal pointer to the queue’s front in order to iterate through the queue again for output
    while (current) {   //looping through the ready queue nodes
        printf("PID %d ", state->pcb->pid);
        current = current->next;    //moving to the next node
    }
    printf("\n");
}*/

/* Sample main for testing
int main() {
    initRoundRobin();
    printf("Enter quantum: ");
    scanf("%d", &quantum);
    setQuantum(quantum);
    PCB* pcb1 = createMockPCB(1, 5); //5 instructions
    PCB* pcb2 = createMockPCB(2, 3); //3 instructions
    PCB* pcb3 = createMockPCB(3, 4); //4 instructions
    enqueue(&readyQueue, pcb1);
    enqueue(&readyQueue, pcb2);
    enqueue(&readyQueue, pcb3);
    // Assume PCBs are added to readyQueue through enqueue during process creation or unblocking
    // Run scheduler until all processes are done step by step
    while (readyQueue.front || runningProcess) {
        printf("\nPress Enter to advance to next cycle...");
        getchar(); // Pause for step-by-step to review outputs (running process and queue contents)
        roundRobinScheduler();
        // Free terminated processes
        if (runningProcess && runningProcess->state == TERMINATED) {
            freePCB(runningProcess);
            runningProcess = NULL;
        }
    }
    while (readyQueue.front) {  //cleaning up any remaining processes after the loop finishes
        PCB* pcb = dequeue(&readyQueue);
        freePCB(pcb);
    }
    return 0;
}*/


// Wrapper struct to track RR-specific fields
typedef struct RRProcess {                      // Defines a custom struct to bundle Process, PCB, and RR-specific data
    Process* process;                           // Points to the Process struct (has pid, file_path, burstTime, etc.)
    PCB* pcb;                                   // Points to the PCB struct (has id, programCounter, mem bounds)
    int timeInQueue;                            // Tracks how many cycles this process waits in the Ready Queue
    int quantumUsed;                            // Counts cycles used in the current quantum for preemption
} RRProcess;

// Global variables
Queue* readyQueue;                              // The Ready Queue holding processes waiting to run
RRProcess* runningProcess = NULL;               // The currently running process (or NULL if none)
int clockCycle = 0;                             // Tracks the current clock cycle of the simulation
int quantum;                                    // Stores the user-defined quantum (time slice) for RR
RRProcess* rrProcesses[20];                    // Array to store all RRProcess instances for tracking
int rrProcessCount = 0;                         // Counts how many RRProcess instances we’ve created

// Initialize the scheduler
void initRoundRobin() {                         // Sets up the scheduler at the start of the simulation
    readyQueue = createQueue();                 // Creates an empty Ready Queue using Queue.c’s function
    runningProcess = NULL;                      // Starts with no running process
    clockCycle = 0;                             // Resets the clock cycle to 0
    rrProcessCount = 0;                         // Resets the count of RRProcess instances
}

// Create an RRProcess wrapper
RRProcess* createRRProcess(int pid, const char* file_path, int arrival_time, int burst_time) { // Makes a new RRProcess for a process, it ensures each process has a Process pcb
    RRProcess* rrProc = (RRProcess*)malloc(sizeof(RRProcess)); //Allocates memory for the RRProcess struct
    if (!rrProc) {                              //Checks if memory allocation failed
        fprintf(stderr, "Memory allocation for RRProcess failed\n"); // Prints an error to the standard error stream (stderr) if allocation fails
        exit(EXIT_FAILURE);                     //Exits the program if we can’t allocate memory with the failure status EXIT_FAILURE
    } 
    rrProc->process = createProcess(pid, file_path, arrival_time, burst_time); // Creates a Process using process.c’s function
    rrProc->pcb = createPCB(pid);               //creates a PCB using PCB.c’s function
    rrProc->timeInQueue = 0;                    //initializing timeInQueue to 0 indicating the process hasn’t waited in the Ready Queue yet
    rrProc->quantumUsed = 0;                    //initializes quantumUsed to 0 indicating the process hasn’t used any cycles in its current quantum (time slice)
    rrProcesses[rrProcessCount++] = rrProc;     //stores the rrProc pointer which is (RRProcess) in the global array for tracking the number of RRProcess instances, so that I can update timeInQueue in the Ready Queue or reuse in scheduling
    return rrProc;
}

// Free an RRProcess
void freeRRProcess(RRProcess* rrProc) {         // Cleans up an RRProcess when a process terminates
    if (rrProc) {                               // Checks if the RRProcess exists
        free(rrProc->process->file_path);        // Frees the file_path string in the Process struct
        free(rrProc->process);                  // Frees the Process struct
        free(rrProc->pcb);                      // Frees the PCB struct
        free(rrProc);                           // Frees the RRProcess itself
    }
}

// Set quantum from user input
void setQuantum(int q) {                        // Sets the quantum (time slice) for RR scheduling
    quantum = (q > 0) ? q : 3;                  // Uses user input if positive; defaults to 3 if invalid
}

// Simulate instruction execution
void executeInstruction(Process* process, PCB* pcb) {     // updates states directly and simulates running one cycle of a process
    //Process* proc = rrProc->process;            // Gets the Process struct from the RRProcess
    if (process->remainingTime > 0) {
        pcb->programCounter++;
        process->remainingTime--;
        if (process->remainingTime == 0) {
            process->state = TERMINATED;
            pcb->state = TERMINATED;
        }
    }
    
    
    /*if (proc->remainingTime > 0) {             // Checks if the process has no time left to run
        return 2;                               // Returns 2 to indicate the process is terminated
    }
    proc->remainingTime--;                      // Decrements remainingTime to simulate one cycle
    rrProc->pcb->programCounter++;              // Increments programCounter to track progress
    return 1; */                                  // Returns 1 to indicate the cycle executed successfully
}

void roundRobinScheduler() {                    //runs one cycle of the RR scheduler
    clockCycle++;                               //increments the clock cycle to track time

    // Update time in Ready Queue
    /*Process* current = readyQueue->front;       //creating a pointer which points to the first process in the Ready Queue
    while (current) {                           //looping through all processes in the queue
        for (int i = 0; i < rrProcessCount; i++) { 
            if (rrProcesses[i] && rrProcesses[i]->process == current) { //checking if the process pointer in the i-th RRProcess matches the current Process from the queue
                rrProcesses[i]->timeInQueue++;  //incrementing timeInQueue for this process (the matched RRProcess)
                break;                          //stops searching once the matching RRProcess is found
            }
        }
        current = current->next;                //moving to the next process in the queue
    }*/

    // Handle running process
    if (runningProcess) {                       // Checks if there’s a process currently running
        runningProcess->quantumUsed++;          // Increments cycles used in this quantum
        executeInstruction(runningProcess->process, runningProcess->pcb); // Executes one cycle and gets status

        if (runningProcess->process->state == TERMINATED) {
            // Remove from rrProcesses to prevent re-enqueue
            for (int i = 0; i < rrProcessCount; i++) {
                if (rrProcesses[i] == runningProcess) {
                    rrProcesses[i] = NULL; // Mark as freed
                    break;
                }
            }
            freeRRProcess(runningProcess);
            runningProcess = NULL;
        /*if (status == 2) {                      // If status is 2, the process has terminated
            runningProcess->process->state = TERMINATED; // Sets Process state to TERMINATED
            runningProcess->pcb->state = TERMINATED; // Sets PCB state to TERMINATED
            freeRRProcess(runningProcess);      // Frees the RRProcess memory
            runningProcess = NULL;              // Clears the running process
        } else if (status == 0) {               // If status is 0, the process is blocked (placeholder)
            runningProcess->process->state = WAITING; // Sets Process state to WAITING
            runningProcess->pcb->state = WAITING; // Sets PCB state to WAITING
            runningProcess = NULL;  */            // Clears the running process (for mutex handling)
        } else if (runningProcess->quantumUsed >= quantum) { // If quantum is used up
            runningProcess->process->state = READY; // Sets Process state to READY
            runningProcess->pcb->state = READY; // Sets PCB state to READY
            runningProcess->quantumUsed = 0;    // Resets quantumUsed for next run
            runningProcess->timeInQueue = 0;
            enqueue(readyQueue, runningProcess->process); // Puts process back in Ready Queue
            //rrProcesses[rrProcessCount++] = runningProcess; //storing RRProcess for reuse
            runningProcess = NULL;              //clearing the running process
        }
    }

    // Schedule next process
    if (!runningProcess && !isEmpty(readyQueue)) { //if no process is running and queue is not empty
        Process* nextProc = dequeue(readyQueue); //dequeuing the first Process* from the Ready Queue storing it in nextProc
        RRProcess* rrProc = NULL;               //initializing a pointer of the type RRProcess
        for (int i = 0; i < rrProcessCount; i++) { //looping the rrProcesses array to find an RRProcess matching nextProc as reusing an existing RRProcess preserves its timeInQueue and quantumUsed will help me avoid redundant allocations
            if (rrProcesses[i] && rrProcesses[i]->process == nextProc) { //checking if the process pointer in the i-th RRProcess matches nextProc ensuring to reuse the same RRProcess to maintain its state
                rrProc = rrProcesses[i];        //reuses the existing RRProcess
                break;                          //stops searching once found
            }
        }
        if (!rrProc) {                          //if no matching RRProcess is found
            rrProc = (RRProcess*)malloc(sizeof(RRProcess)); //allocates memory for a new RRProcess assigns the pointer to rrProc
            rrProc->process = nextProc;         //assigning the dequeued Process to the process fiekd
            rrProc->pcb = createPCB(nextProc->pid); //creating a new PCB for it
            rrProc->timeInQueue = 0;            //initializes timeInQueue to 0
            rrProc->quantumUsed = 0;            //initializes quantumUsed to 0
            rrProcesses[rrProcessCount++] = rrProc; //storing the new RRProcess (rrProc) in the rrProceses array at index rrProcessCount then increments rrProcessCount to track the number of RRProcess instances
        }
        runningProcess = rrProc;                //setting the RRProcess for nextProc as the running process
        runningProcess->process->state = RUNNING; //setting Process state to RUNNING
        runningProcess->pcb->state = RUNNING;   //setting PCB state to RUNNING
        //runningProcess->timeInQueue = 0; // Reset timeInQueue when running
        executeInstruction(runningProcess->process, runningProcess->pcb); // Execute first instruction
    }
    //timeInQueue is updated after scheduling, so only processes in readyQueue (after dequeuing the running process) are incremented.

    // Update time in Ready Queue
    Process* current = readyQueue->front;       //creating a pointer which points to the first process in the Ready Queue
    while (current) {                           //looping through all processes in the queue
        for (int i = 0; i < rrProcessCount; i++) { 
            if (rrProcesses[i] && rrProcesses[i]->process == current) { //checking if the process pointer in the i-th RRProcess matches the current Process from the queue
                rrProcesses[i]->timeInQueue++;  //incrementing timeInQueue for this process (the matched RRProcess)
                break;                          //stops searching once the matching RRProcess is found
            }
        }
        current = current->next;                //moving to the next process in the queue
    }


    // Output for GUI integration
    printf("Clock Cycle %d: ", clockCycle);     // Prints the current clock cycle
    if (runningProcess) {                       // Checks if there’s a running process
        printf("Running PID %d, PC %d, timeInQueue %d\n", // Shows PID, programCounter, and timeInQueue
               runningProcess->process->pid, runningProcess->pcb->programCounter,
               runningProcess->timeInQueue);
    } else {                                    // If no process is running
        printf("No running process\n");         // Prints that no process is running
    }
    printf("Ready ");                    // Starts printing the Ready Queue
    display(readyQueue);                        // Uses Queue.c’s display to show queue contents
}

// Sample main for testing
int main() {                                    // Main function to test the scheduler
    initRoundRobin();                           // Initializes the scheduler
    printf("Enter quantum: ");                  // Asks user for the quantum value
    scanf("%d", &quantum);                      // Reads the quantum input
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Clear input buffer
    setQuantum(quantum);                        // Sets the quantum (or defaults to 3)

    // Create test processes
    RRProcess* p1 = createRRProcess(1, "p1.txt", 0, 5); // Creates process P1 with 5 cycles
    RRProcess* p2 = createRRProcess(2, "p2.txt", 0, 3); // Creates process P2 with 3 cycles
    RRProcess* p3 = createRRProcess(3, "p3.txt", 0, 4); // Creates process P3 with 4 cycles

    // Enqueue processes
    enqueue(readyQueue, p1->process);           // Adds P1 to the Ready Queue
    enqueue(readyQueue, p2->process);           // Adds P2 to the Ready Queue
    enqueue(readyQueue, p3->process);           // Adds P3 to the Ready Queue

    // Run scheduler
    while (!isEmpty(readyQueue) || runningProcess) { // Loops until queue is empty and no process runs
        printf("\nPress Enter to advance to next cycle..."); // Pauses for user input
        getchar();                              // Waits for Enter key
        roundRobinScheduler();                  // Runs one cycle of the scheduler
    }

    // Clean up
    freeQueue(readyQueue);                      // Frees the Ready Queue and its processes
    for (int i = 0; i < rrProcessCount; i++) {  // Loops through RRProcess array
        if (rrProcesses[i]) {                   // Checks if RRProcess exists
            freeRRProcess(rrProcesses[i]);      // Frees each RRProcess
        }
    }
    return 0;                                   // Exits the program
}
