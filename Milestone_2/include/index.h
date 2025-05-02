// index.h
#ifndef INDEX_H
#define INDEX_H

#include "uthash.h"

typedef struct {
    char *key;          // Key: e.g., "P1_Instruction_1"
    int address;        // Value: Memory address (0–59)
    UT_hash_handle hh;  // uthash handle
} IndexEntry;

void initIndex(IndexEntry **index);
void addIndexEntry(IndexEntry **index, const char *key, int address);
int getIndexAddress(IndexEntry *index, const char *key);
void freeIndex(IndexEntry **index);

#endif // INDEX_H