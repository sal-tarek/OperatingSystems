// memory.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

void initMemory(MemoryWord **memory) {
    *memory = NULL; // Initialize empty hashmap
}

void addMemoryData(MemoryWord **memory, int address, const char *data) {
    if (address < 0 || address > 59) {
        fprintf(stderr, "Invalid memory address: %d\n", address);
        return;
    }

    // Check if address already exists
    MemoryWord *entry;
    HASH_FIND_INT(*memory, &address, entry);
    if (entry) {
        // Update existing entry
        free(entry->data);
        entry->data = strdup(data);
    } else {
        // Create new entry
        entry = (MemoryWord*)malloc(sizeof(MemoryWord));
        if (!entry) {
            fprintf(stderr, "Memory allocation for MemoryWord failed\n");
            exit(EXIT_FAILURE);
        }
        entry->address = address;
        entry->data = strdup(data);
        if (!entry->data) {
            fprintf(stderr, "Failed to allocate memory for data\n");
            free(entry);
            exit(EXIT_FAILURE);
        }
        HASH_ADD_INT(*memory, address, entry);
    }
}

char* getMemoryData(MemoryWord *memory, int address) {
    if (address < 0 || address > 59) {
        fprintf(stderr, "Invalid memory address: %d\n", address);
        return NULL;
    }

    MemoryWord *entry;
    HASH_FIND_INT(memory, &address, entry);
    return entry ? entry->data : NULL;
}

void freeMemory(MemoryWord **memory) {
    MemoryWord *entry, *tmp;
    HASH_ITER(hh, *memory, entry, tmp) {
        HASH_DEL(*memory, entry);
        free(entry->data);
        free(entry);
    }
    *memory = NULL;
}

void printMemory(MemoryWord *memory) {
    MemoryWord *curr, *tmp;
    printf("Memory Contents:\n");
    HASH_ITER(hh, memory, curr, tmp) {
        printf("Address: %d => Data: %s\n", curr->address, curr->data);
    }
}