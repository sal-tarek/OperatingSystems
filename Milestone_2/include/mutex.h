#ifndef MUTEX_H
#define MUTEX_H
#include <stdbool.h>
#include "PCB.h"
#include "memory_manager.h"
#include "Queue.h"

#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support
#define MAX_NUM_QUEUES 4        // Maximum number of queues

// Mutex structure (opaque pointer)
typedef struct mutex_t mutex_t;

// Global general blocked queue
extern Queue *global_blocked_queue;
extern Queue *readyQueues[MAX_NUM_QUEUES];

// Function prototypes
void mutex_init_system(void);
int mutex_lock(mutex_t *mutex, Process *process);
int mutex_unlock(mutex_t *mutex, Process *process);
mutex_t* get_mutex_by_name(const char* name);
void cleanup_process_mutexes(Process *process);

#endif // MUTEX_H
