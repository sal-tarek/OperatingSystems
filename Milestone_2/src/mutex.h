#ifndef MUTEX_H
#define MUTEX_H
#include <stdbool.h>
#include "PCB.h"
#include "memory_manager.h"
#include "Queue.h"

// Maximum processes that can wait for a resource
#define MAX_BLOCKED_PROCESSES 10
#define MAX_GLOBAL_BLOCKED 3

// Mutex structure (opaque pointer)
typedef struct mutex_t mutex_t;

// Global mutex instances
extern mutex_t userInput_mutex;
extern mutex_t userOutput_mutex;
extern mutex_t file_mutex;

// Global general blocked queue
extern Queue *global_blocked_queue;
extern Queue *readyQueues[4];

// Function prototypes
void mutex_init_system(void);
int mutex_lock(mutex_t *mutex, Process *process);
int mutex_unlock(mutex_t *mutex, Process *process);
mutex_t* get_mutex_by_name(const char* name);
void cleanup_process_mutexes(Process *process);

#endif // MUTEX_H
