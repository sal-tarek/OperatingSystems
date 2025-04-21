#include <stdlib.h>
#include "PCB.h"
#include <stdio.h>
#include "memory_manager.h"
#include "index.h"


struct PCB* createPCB(int pid) {
    struct PCB *pcb = (struct PCB*)malloc(sizeof(struct PCB));
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
    struct PCB *pcb = (struct PCB*)malloc(sizeof(struct PCB));
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

void freePCB(struct PCB *pcb) {
    if (pcb) {
        free(pcb);
    }
}

int getPCBId(struct PCB *pcb) {
    return pcb ? pcb->id : -1;
}

ProcessState getPCBState(struct PCB *pcb) {
    return pcb ? pcb->state : NEW;
}

int getPCBPriority(struct PCB *pcb) {
    return pcb ? pcb->priority : 0;
}

int getPCBProgramCounter(struct PCB *pcb) {
    return pcb ? pcb->programCounter : 0;
}

int getPCBMemLowerBound(struct PCB *pcb) {
    return pcb ? pcb->memLowerBound : 0;
}

int getPCBMemUpperBound(struct PCB *pcb) {
    return pcb ? pcb->memUpperBound : 0;
}

void setPCBState(struct PCB *pcb, ProcessState state) {
    if (pcb) {
        pcb->state = state;
    }
}

void setPCBPriority(struct PCB *pcb, int priority) {
    if (pcb) {
        pcb->priority = priority;
    }
}

void setPCBProgramCounter(struct PCB *pcb, int pc) {
    if (pcb) {
        pcb->programCounter = pc;
    }
}

void setPCBMemLowerBound(struct PCB *pcb, int memLowerBound) {
    if (pcb) {
        pcb->memLowerBound = memLowerBound;
    }
}

void setPCBMemUpperBound(struct PCB *pcb, int memUpperBound) {
    if (pcb) {
        pcb->memUpperBound = memUpperBound;
    }
}

//if needed (for debugging maybe)
void printPCB(struct PCB* pcb) {
    printf("PCB Info:\n");
    printf("ID: %d\n", pcb->id);
    printf("State: %d\n", pcb->state);
    printf("PC: %d\n", pcb->programCounter);
    printf("Priority: %d\n", pcb->priority);
    printf("Memory Bounds: [%d, %d]\n", pcb->memLowerBound, pcb->memUpperBound);
}



