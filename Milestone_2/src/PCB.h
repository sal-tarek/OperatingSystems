#ifndef PCB_H
#define PCB_H

#include "process.h"  // for ProcessState

typedef struct PCB {
    int id;                     // Unique ID for PCB
    ProcessState state;         // State of the process
    int priority;               // Priority (optional feature)
    int programCounter;         // Tracks execution progress
    int memLowerBound;          // Start of memory allocated
    int memUpperBound;          // End of memory allocated
} PCB;

PCB* createPCB(int id);         // Create PCB with given ID

#endif // PCB_H
