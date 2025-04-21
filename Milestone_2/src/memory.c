#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memory.h"
#include "PCB.h"
#include "uthash.h"

// Global variable declared in main.c
extern MemoryWord *memory;

void addMemoryData(MemoryWord **memory, int address, void *data, DataType type) {
    MemoryWord *word = NULL;
    HASH_FIND_INT(*memory, &address, word);
    if (word == NULL) {
        word = (MemoryWord*)malloc(sizeof(MemoryWord));
        word->address = address;
        word->data = NULL;
        word->type = type;
        HASH_ADD_INT(*memory, address, word);
    } else {
        if (word->type == TYPE_STRING) {
            free(word->data);
        } else if (word->type == TYPE_PCB) {
            free((struct PCB*)word->data);
        }
        word->type = type;
    }

    if (type == TYPE_STRING) {
        word->data = strdup((char*)data);
    } else if (type == TYPE_PCB) {
        word->data = data; // Store PCB* directly
    }
}

void* getMemoryData(MemoryWord *memory, int address) {
    MemoryWord *word = NULL;
    HASH_FIND_INT(memory, &address, word);
    return word ? word->data : NULL;
}

int updateMemoryData(MemoryWord **memory, int address, void *new_data, DataType type) {
    MemoryWord *word = NULL;
    HASH_FIND_INT(*memory, &address, word);
    if (word == NULL) {
        return -1;
    }
    if (word->type == TYPE_STRING) {
        free(word->data);
    } else if (word->type == TYPE_PCB) {
        free((struct PCB*)word->data);
    }
    word->type = type;
    if (type == TYPE_STRING) {
        word->data = strdup((char*)new_data);
    } else if (type == TYPE_PCB) {
        word->data = new_data;
    }
    return 0;
}

void freeMemoryWord() {
    MemoryWord *current, *tmp;
    HASH_ITER(hh, memory, current, tmp) {
        HASH_DEL(memory, current);
        if (current->type == TYPE_STRING) {
            free(current->data);
        } else if (current->type == TYPE_PCB) {
            free((struct PCB*)current->data);
        }
        free(current);
    }
}

void printMemory() {
    MemoryWord *curr, *tmp;
    printf("Memory Contents:\n");
    HASH_ITER(hh, memory, curr, tmp) {
        printf("Address: %d => Type: %s, Data: ", curr->address, 
               curr->type == TYPE_STRING ? "STRING" : "PCB");
        if (curr->data == NULL) {
            printf("NULL\n");
            continue;
        }
        if (curr->type == TYPE_STRING) {
            printf("%s\n", (char*)curr->data);
        } else if (curr->type == TYPE_PCB) {
            struct PCB *pcb = (struct PCB*)curr->data;
            const char *state_str;
            switch (getPCBState(pcb)) {
                case NEW: state_str = "NEW"; break;
                case READY: state_str = "READY"; break;
                case RUNNING: state_str = "RUNNING"; break;
                case WAITING: state_str = "WAITING"; break;
                case TERMINATED: state_str = "TERMINATED"; break;
                default: state_str = "UNKNOWN";
            }
            printf("PID=%d, State=%s, Priority=%d, PC=%d, MemLower=%d, MemUpper=%d\n",
                   getPCBId(pcb), state_str, getPCBPriority(pcb),
                   getPCBProgramCounter(pcb), getPCBMemLowerBound(pcb),
                   getPCBMemUpperBound(pcb));
        }
    }
}