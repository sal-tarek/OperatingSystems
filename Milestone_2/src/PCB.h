#ifndef PCB_H
#define PCB_H

#include "process.h"

struct PCB {
    int id;                // Process ID
    ProcessState state;    // Process state
    int priority;          // Priority
    int programCounter;    // Program counter
    int memLowerBound;     // Lower memory bound
    int memUpperBound;     // Upper memory bound
};

struct PCB* createPCB(int pid);
struct PCB* createPCBWithBounds(int pid, int memLowerBound, int memUpperBound);
void freePCB(struct PCB *pcb);

// Getters
int getPCBId(struct PCB *pcb);
ProcessState getPCBState(struct PCB *pcb);
int getPCBPriority(struct PCB *pcb);
int getPCBProgramCounter(struct PCB *pcb);
int getPCBMemLowerBound(struct PCB *pcb);
int getPCBMemUpperBound(struct PCB *pcb);

// Setters
void setPCBState(struct PCB *pcb, ProcessState state);
void setPCBPriority(struct PCB *pcb, int priority);
void setPCBProgramCounter(struct PCB *pcb, int pc);
void setPCBMemLowerBound(struct PCB *pcb, int memLowerBound);
void setPCBMemUpperBound(struct PCB *pcb, int memUpperBound);

#endif // PCB_H