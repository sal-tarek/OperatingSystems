#ifndef PCB_H
#define PCB_H

#include "process.h"


typedef struct {
    int id;                // Process ID
    ProcessState state;    // Process state
    int priority;          // Priority
    int programCounter;    // Program counter
    int memLowerBound;     // Lower memory bound
    int memUpperBound;     // Upper memory bound
} PCB;  // Now we can use "PCB" instead of "struct PCB"


PCB* createPCB(int pid);
PCB* createPCBWithBounds(int pid, int memLowerBound, int memUpperBound);
void freePCB(PCB *pcb);

// Getters
int getPCBId( PCB *pcb);
ProcessState getPCBState( PCB *pcb);
int getPCBPriority( PCB *pcb);
int getPCBProgramCounter( PCB *pcb);
int getPCBMemLowerBound(PCB *pcb);
int getPCBMemUpperBound(  PCB *pcb);

// Setters
void setPCBState( PCB *pcb, ProcessState state);
void setPCBPriority( PCB *pcb, int priority);
void setPCBProgramCounter( PCB *pcb, int pc);
void setPCBMemLowerBound( PCB *pcb, int memLowerBound);
void setPCBMemUpperBound( PCB *pcb, int memUpperBound);
void printPCB( PCB *pcb);

#endif // PCB_H
