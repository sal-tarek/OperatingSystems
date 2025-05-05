#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
extern IndexEntry *index_table;

void initIndex() {
    index_table = NULL;
}

void addIndexEntry(const char *key, int address) {
    if (address < 0 || address > 59) {
        fprintf(stderr, "Invalid memory address: %d\n", address);
        return;
    }

    IndexEntry *entry;
    HASH_FIND_STR(index_table, key, entry);
    if (entry) {
        entry->address = address;
    } else {
        entry = (IndexEntry*)malloc(sizeof(IndexEntry));
        if (!entry) {
            fprintf(stderr, "Memory allocation for IndexEntry failed\n");
            exit(EXIT_FAILURE);
        }
        entry->key = strdup(key);
        if (!entry->key) {
            fprintf(stderr, "Failed to allocate memory for key\n");
            free(entry);
            exit(EXIT_FAILURE);
        }
        entry->address = address;
        HASH_ADD_STR(index_table, key, entry);
    }
}

int getIndexAddress(const char *key) {
    IndexEntry *entry;
    HASH_FIND_STR(index_table, key, entry);
    return entry ? entry->address : -1; // -1 for not found
}

void freeIndex() {
    IndexEntry *entry, *tmp;
    HASH_ITER(hh, index_table, entry, tmp) {
        HASH_DEL(index_table, entry);
        free(entry->key);
        free(entry);
    }
    index_table = NULL;
}