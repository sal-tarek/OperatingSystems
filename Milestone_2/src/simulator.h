#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Queue.h"
#include "memory.h"
#include "index.h"

// Global variables
extern Queue *job_pool;
extern MemoryWord *memory;
extern IndexEntry *index;

#endif // SIMULATOR_H