#include <stdlib.h>
#include "PCB.h"
#include <stdio.h>

struct PCB* createPCB(int pid) {
     PCB *pcb = ( PCB*)malloc(sizeof( PCB));
    if (pcb == NULL) {
        return NULL;
    }
    pcb->id = pid;
    pcb->state = NEW;
    pcb->priority = 0;
    pcb->programCounter = 0;
    pcb->memLowerBound = 0;
    pcb->memUpperBound = 0;
    return pcb;
}

struct PCB* createPCBWithBounds(int pid, int memLowerBound, int memUpperBound) {
     PCB *pcb = ( PCB*)malloc(sizeof( PCB));
    if (pcb == NULL) {
        return NULL;
    }
    pcb->id = pid;
    pcb->state = NEW;
    pcb->priority = 0;
    pcb->programCounter = 0;
    pcb->memLowerBound = memLowerBound;
    pcb->memUpperBound = memUpperBound;
    return pcb;
}

void freePCB( PCB *pcb) {
    if (pcb) {
        free(pcb);
    }
}

int getPCBId( PCB *pcb) {
    return pcb ? pcb->id : -1;
}

ProcessState getPCBState( PCB *pcb) {
    return pcb ? pcb->state : NEW;
}

int getPCBPriority( PCB *pcb) {
    return pcb ? pcb->priority : 0;
}

int getPCBProgramCounter( PCB *pcb) {
    return pcb ? pcb->programCounter : 0;
}

int getPCBMemLowerBound( PCB *pcb) {
    return pcb ? pcb->memLowerBound : 0;
}

int getPCBMemUpperBound( PCB *pcb) {
    return pcb ? pcb->memUpperBound : 0;
}

void setPCBState( PCB *pcb, ProcessState state) {
    if (pcb) {
        pcb->state = state;
    }
}

void setPCBPriority( PCB *pcb, int priority) {
    if (pcb) {
        pcb->priority = priority;
    }
}

void setPCBProgramCounter( PCB *pcb, int pc) {
    if (pcb) {
        pcb->programCounter = pc;
    }
}

void setPCBMemLowerBound( PCB *pcb, int memLowerBound) {
    if (pcb) {
        pcb->memLowerBound = memLowerBound;
    }
}

void setPCBMemUpperBound( PCB *pcb, int memUpperBound) {
    if (pcb) {
        pcb->memUpperBound = memUpperBound;
    }
}

void printPCB( PCB* pcb) {
    printf("PCB Info:\n");
    printf("ID: %d\n", pcb->id);
    printf("State: %d\n", pcb->state);
    printf("PC: %d\n", pcb->programCounter);
    printf("Priority: %d\n", pcb->priority);
    printf("Memory Bounds: [%d, %d]\n", pcb->memLowerBound, pcb->memUpperBound);
}
