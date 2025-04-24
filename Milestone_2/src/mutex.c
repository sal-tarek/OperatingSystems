#include <stdio.h>
#include <string.h> 
#include "mutex.h"
#include <stdbool.h>
#include <stdlib.h>


// Mutex structure definition
struct mutex_t {
    char name[20];
    bool available;
    PCB *
    holder;
    PCB *blocked_queue[MAX_BLOCKED_PROCESSES];
    int blocked_count;
};

// Global mutex instances
mutex_t userInput_mutex = {"userInput", true, NULL, {0}, 0};
mutex_t userOutput_mutex = {"userOutput", true, NULL, {0}, 0};
mutex_t file_mutex = {"file", true, NULL, {0}, 0};

// Global general blocked queue
PCB *global_blocked_queue[MAX_GLOBAL_BLOCKED];
int global_blocked_count = 0;
void add_to_global_blocked_queue(PCB *process) {
    if (global_blocked_count < MAX_GLOBAL_BLOCKED) {
        global_blocked_queue[global_blocked_count++] = process;
    } else {
        fprintf(stderr, "Error: Global blocked queue full\n");
    }
}

void remove_from_global_blocked_queue(PCB *process) {
    for (int i = 0; i < global_blocked_count; i++) {
        if (global_blocked_queue[i] == process) {
            for (int j = i; j < global_blocked_count - 1; j++) {
                global_blocked_queue[j] = global_blocked_queue[j + 1];
            }
            global_blocked_count--;
            break;
        }
    }
}

// Initialize all mutexes
void mutex_init_system(void) {
    userInput_mutex.available = true;
    userInput_mutex.holder = NULL;
    userInput_mutex.blocked_count = 0;

    userOutput_mutex.available = true;
    userOutput_mutex.holder = NULL;
    userOutput_mutex.blocked_count = 0;

    file_mutex.available = true;
    file_mutex.holder = NULL;
    file_mutex.blocked_count = 0;
}

// Lock a mutex (semWait)
int mutex_lock(mutex_t *mutex, PCB *process) {
    if (mutex == NULL || process == NULL) {
        fprintf(stderr, "Error: Null parameter in mutex_lock\n");
        return 1;  // Return 1 for error
    }

    if (mutex->available) {
        // Mutex is available, acquire it
        mutex->available = false;
            process->state = BLOCKED;  // Use enum value
        mutex->holder = process;  // Assign the process as the mutex holder
        process->state = RUNNING;  // Use enum value
        return 0;  // Success
    } else {
        // Mutex is not available, block the process
        if (mutex->blocked_count < MAX_BLOCKED_PROCESSES) {
            mutex->blocked_queue[mutex->blocked_count++] = process;
            process->state = BLOCKED;  // Use enum value
            add_to_global_blocked_queue(process);

            printf("Process %d blocked waiting for %s\n", process->id, mutex->name);
        } else {
            fprintf(stderr, "Error: Blocked queue full for %s\n", mutex->name);
        }
    }
    return 1;  // Failure
}

// Unlock a mutex (semSignal)
int mutex_unlock(mutex_t *mutex, PCB *process) {
    if (mutex == NULL || process == NULL) {
        fprintf(stderr, "Error: Null parameter in mutex_unlock\n");
        return 1;  // Return 1 for error
    }

    if (!mutex->available && mutex->holder == process) {
        // Release the mutex
        mutex->available = true;
        mutex->holder = NULL;

        // Unblock the highest priority process waiting for this mutex
        if (mutex->blocked_count > 0) {
            int highest_pri = -1;
            int selected_idx = -1;

            // Find highest priority process in blocked queue
            for (int i = 0; i < mutex->blocked_count; i++) {
                if (mutex->blocked_queue[i]->priority > highest_pri) {
                    highest_pri = mutex->blocked_queue[i]->priority;
                    selected_idx = i;
                }
            }

            if (selected_idx != -1) {
                PCB *next_process = mutex->blocked_queue[selected_idx];

                // Remove from blocked queue
                for (int i = selected_idx; i < mutex->blocked_count - 1; i++) {
                    mutex->blocked_queue[i] = mutex->blocked_queue[i + 1];
                }
                mutex->blocked_count--;
                remove_from_global_blocked_queue(next_process);


                // Acquire the mutex for this process
                mutex->available = false;
                mutex->holder = next_process;
                next_process->state =READY;
                printf("Process %d unblocked for %s\n", next_process->id, mutex->name);
            }
        }
        return 0;  // Success
    } else if (mutex->available) {
        fprintf(stderr, "Warning: mutex_unlock called on available mutex %s\n", mutex->name);
    } else {
        fprintf(stderr, "Error: Process %d doesn't hold mutex %s\n", 
                process->id, mutex->name);
    }
    return 1;  // Failure
}

// Get mutex by resource name
mutex_t* get_mutex_by_name(const char* name) {
    if (strcmp(name, "userInput") == 0) return &userInput_mutex;
    if (strcmp(name, "userOutput") == 0) return &userOutput_mutex;
    if (strcmp(name, "file") == 0) return &file_mutex;
    return NULL;
}

// Clean up when a process terminates
void cleanup_process_mutexes(PCB *process) {
    if (process == NULL) return;

    if (userInput_mutex.holder == process) {
        mutex_unlock(&userInput_mutex, process);
    }
    if (userOutput_mutex.holder == process) {
        mutex_unlock(&userOutput_mutex, process);
    }
    if (file_mutex.holder == process) {
        mutex_unlock(&file_mutex, process);
    }}
    int main() {
        printf("Mutex System Initialization...\n");
    
    
        return 0;
    }
    
