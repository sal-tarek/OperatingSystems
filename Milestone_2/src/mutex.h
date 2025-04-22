#ifndef MUTEX_H
#define MUTEX_H
#include "PCB.h"
#include <stdbool.h>

// Maximum processes that can wait for a resource
#define MAX_BLOCKED_PROCESSES 10

// Mutex structure (opaque pointer)
typedef struct mutex_t mutex_t;

// Global mutex instances
extern mutex_t userInput_mutex;
extern mutex_t userOutput_mutex;
extern mutex_t file_mutex;

// Function prototypes
void mutex_init_system(void);
int mutex_lock(mutex_t *mutex, PCB *process);
int mutex_unlock(mutex_t *mutex, PCB *process);
mutex_t* get_mutex_by_name(const char* name);
void cleanup_process_mutexes(PCB *process);

#endif // MUTEX_H
