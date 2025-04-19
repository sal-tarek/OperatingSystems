#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PCB {
    int processID;
    int programCounter;
    int instructionCount;
    char** instructions;
    //Assumed other fields for PCB are declared by the person responsible for PCB.
    int state; // 0=RUNNING, 1=READY, 2=BLOCKED, 3=TERMINATED OR it can be enum depending on the person who implemented PCB.
    int timeInQueue; // Added for RR, it tracks the time (in clock cycles) the process has spent in the Ready Queue
    int quantumUsed; // Added for RR, it tracks the number of clock cycles the process has used in its current quantum
} PCB;

// Node for Ready Queue (linked list)
typedef struct QueueNode {
    PCB* pcb;                  //I'm storing a pointer to the PCB of the process in this node
    struct QueueNode* next;    // this points to the next node in the linked list
} QueueNode;

// This is the structure of the Ready Queue
typedef struct {
    QueueNode* front;   //points to the first node in the queue
    QueueNode* rear;    //points to the last node in the queue
} Queue;

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
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));        //allocating a memory for a new queue node
    node->pcb = pcb;        //setting the node's PCB pointer to the inputted PCB
    node->next = NULL;      //setting the node's next pointer to NULL indicating that this node will be the last in the queue until another one is added
    if (q->rear == NULL) {  //here I'm checking if the queue is empty
        q->front = q->rear = node;      //if the rear is NULL then the queue has no nodes so the new node becomes both the front and rear
    } else {
        q->rear->next = node;       //setting the "next" pointer of the last node to the new node
        q->rear = node;             //updating the rear poimter to the new node, making the new node ready for the next enqueue
    }
    pcb->timeInQueue = 0;           //resetting the process's time in the ready queue
    pcb->state = 1; // setting the state to READY
}

// Dequeue a process from Ready Queue
PCB* dequeue(Queue* q) {        //here I'm removing and returning the PCB at the front of the ready queue
    if (q->front == NULL) return NULL;  //if the front is NULL thus no processes to dequeue so return NULL
    QueueNode* temp = q->front;     //storing the front node in a temp pointer, saving the node to be removed so that I can access its PCB later if needed
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

void freePCB(PCB* pcb) {    //this shouls be called when the process terminates
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

void roundRobinScheduler() {
    clockCycle++;

    // Update time in Ready Queue
    QueueNode* current = readyQueue.front;      //this is a pointer to traverse the ready queue, starts at the front of the queue to update the timeInQueue for all ready processes
    while (current) {       //loops through all nodes in the ready qeue
        current->pcb->timeInQueue++;        //increments the waiting time for a process in the ready queue by accessing the pcb field then the timeInQueue field
        current = current->next;    //moves to the next node in the queue
    }

    // Handle running process
    if (runningProcess) {       //checks if a process is currently running, if runningProcess is not NULL then I'll execute its next instr. and check its status
        runningProcess->quantumUsed++;      //incrementing the cycles used by the running process in its current quantum in order to track how long the process has run to compare against "quantum" for preemption
        int status = executeInstruction(runningProcess);    //executes 1 instr of the running process

        if (status == 2) { // checks if process has terminated, Status 2 indicates all instructions are executed
            runningProcess->state = 3;  //Sets the process’s state to Terminated (marking the process as complete)
            runningProcess = NULL;  //indicates no process is running, allowing the scheduler to select a new process
        } else if (status == 0) { // Blocked
            runningProcess->state = 2;  //Sets the process’s state to Blocked.
            runningProcess = NULL; // allowing the scheduler to move to the next process, handled by mutex person
        } else if (runningProcess->quantumUsed >= quantum) { // checks if process's quantum expired
            runningProcess->state = 1;  //setting the process state to Ready, preparing it to be requeued
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
        printf("Running PID %d, PC %d\n", runningProcess->processID, runningProcess->programCounter);
    } else {
        printf("No running process\n");
    }
    printf("Ready Queue: ");
    current = readyQueue.front; //Resetting the traversal pointer to the queue’s front in order to iterate through the queue again for output
    while (current) {   //looping through the ready queue nodes
        printf("PID %d ", current->pcb->processID);
        current = current->next;    //moving to the next node
    }
    printf("\n");
}

// Sample main for testing
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
        if (runningProcess && runningProcess->state == 3) {
            freePCB(runningProcess);
            runningProcess = NULL;
        }
    }
    while (readyQueue.front) {  //cleaning up any remaining processes after the loop finishes
        PCB* pcb = dequeue(&readyQueue);
        freePCB(pcb);
    }
    return 0;
}