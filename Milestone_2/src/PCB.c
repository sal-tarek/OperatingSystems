#include <stdio.h>
#include <stdlib.h>
#include "PCB.h"  

PCB* createPCB(int id) {
    PCB* newPCB = (PCB*)malloc(sizeof(PCB));
    if (!newPCB) {
        fprintf(stderr, "Memory allocation for PCB failed\n");
        exit(EXIT_FAILURE);
    }

    newPCB->id = id;
    newPCB->state = NEW;
    newPCB->priority = 0;
    newPCB->programCounter = 0;
    newPCB->memLowerBound = 0;
    newPCB->memUpperBound = 0;

    return newPCB;
}

void freePCB(PCB* pcb) {
    if (pcb) {
        free(pcb);
    }
}
void updatePCBState(PCB* pcb, ProcessState newState) {
    if (pcb) {
        pcb->state = newState;
    }
}
void updatePCBProgramCounter(PCB* pcb, int newPC) {
    if (pcb) {
        pcb->programCounter = newPC;
    }
}

//if needed (for debugging maybe)
void printPCB(const PCB* pcb) {
    printf("PCB Info:\n");
    printf("ID: %d\n", pcb->id);
    printf("State: %d\n", pcb->state);
    printf("PC: %d\n", pcb->programCounter);
    printf("Priority: %d\n", pcb->priority);
    printf("Memory Bounds: [%d, %d]\n", pcb->memLowerBound, pcb->memUpperBound);
}
