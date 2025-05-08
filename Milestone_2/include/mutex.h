#ifndef MUTEX_H
#define MUTEX_H

#include <stdbool.h>
#include "PCB.h"
#include "memory_manager.h"
#include "Queue.h"

#define MAX_NUM_PROCESSES 10
#define MAX_NUM_QUEUES 4

typedef struct mutex_t {
    char name[20];
    bool available;
    Process *holder;
    Process *blocked_queue[MAX_NUM_PROCESSES];
    int blocked_count;
} mutex_t;

// Global mutex instances
extern mutex_t userInput_mutex;
extern mutex_t userOutput_mutex;
extern mutex_t file_mutex;

// Global queues
extern Queue *global_blocked_queue;
extern Queue *readyQueues[MAX_NUM_QUEUES];

// Function prototypes
void reset_all_mutexes();
int mutex_lock(mutex_t *mutex, Process *process);
int mutex_unlock(mutex_t *mutex, Process *process);
mutex_t *get_mutex_by_name(const char *name);
void cleanup_process_mutexes(Process *process);
Process *checkUnblocked(char *resource_name);

// Accessor functions
bool mutex_is_available(const mutex_t *mutex);
const char *mutex_get_name(const mutex_t *mutex);
Process *mutex_get_holder(const mutex_t *mutex);
int mutex_get_blocked_count(const mutex_t *mutex);
Process *mutex_get_blocked_process(const mutex_t *mutex, int index);

#endif // MUTEX_H