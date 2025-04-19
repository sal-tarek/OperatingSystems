// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include "uthash.h"

typedef struct {
    int address;        // Key: Memory address (0 to 59)
    char *data;         // Value: Data (e.g., "line1", "var1", "PID:1")
    UT_hash_handle hh;  // uthash handle for hash table
} MemoryWord;

#endif // MEMORY_H