#ifndef MEMORY_H
#define MEMORY_H

#include "uthash.h"

typedef enum { TYPE_STRING, TYPE_PCB } DataType;

typedef struct {
    int address;        // Key: Memory address (0 to 59)
    void *data;         // Data: char* for instructions/variables, PCB* for PCB
    DataType type;      // Type of data
    UT_hash_handle hh;  // uthash handle for hash table
} MemoryWord;

void addMemoryData(MemoryWord **memory, int address, void *data, DataType type);
void* getMemoryData(MemoryWord *memory, int address);
int updateMemoryData(MemoryWord **memory, int address, void *new_data, DataType type);
void freeMemoryWord();
void printMemory();

#endif // MEMORY_H