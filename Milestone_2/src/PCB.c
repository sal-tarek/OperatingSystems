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
