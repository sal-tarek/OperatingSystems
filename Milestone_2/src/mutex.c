#include <stdio.h>
#include <string.h>
#include "mutex.h"
#include <stdbool.h>
#include <stdlib.h>
#include "Queue.h"
#include "process.h"

// Global mutex instances
mutex_t userInput_mutex = {"userInput", true, NULL, {0}, 0};
mutex_t userOutput_mutex = {"userOutput", true, NULL, {0}, 0};
mutex_t file_mutex = {"file", true, NULL, {0}, 0};

// Global queues


Process *checkUnblocked(char *resource_name)
{
    mutex_t *mutex = get_mutex_by_name(resource_name);

    if (mutex && !mutex->available && mutex->holder)
    {
        for (int i = 0; i < MAX_NUM_QUEUES; i++)
        {
            if (readyQueues[i]) {
                Process *temp = readyQueues[i]->front;
                while (temp)
                {
                    if (temp->pid == mutex->holder->pid && temp->state == READY)
                    {
                        return temp;
                    }
                    temp = temp->next;
                }
            }
        }
    }
    return NULL;
}

void remove_from_global_blocked_queue(Process *process)
{
    if (!global_blocked_queue || isEmpty(global_blocked_queue)) return;

    printf("Before: Global Blocked ");
    displayQueueSimplified(global_blocked_queue);

    if (process->pid == global_blocked_queue->front->pid )
    {
        Process *p = dequeue(global_blocked_queue);
        if (p && process-> state != TERMINATED) {
            enqueue(readyQueues[getProcessPriority(p->pid)], p);
        }
        printf("After: Global Blocked ");
        displayQueueSimplified(global_blocked_queue);
        return;
    }

    Queue *temp_queue = createQueue();
    while (!isEmpty(global_blocked_queue))
    {
        Process *p = dequeue(global_blocked_queue);
        if (p != process) {
            enqueue(temp_queue, p);
        } else {
            enqueue(readyQueues[getProcessPriority(p->pid)], p);
        }
    }
    
    while (!isEmpty(temp_queue))
    {
        enqueue(global_blocked_queue, dequeue(temp_queue));
    }
    free(temp_queue);
}

void reset_all_mutexes() {
    // Reset the three global mutexes
    userInput_mutex.available = true;
    userInput_mutex.holder = NULL;
    userInput_mutex.blocked_count = 0;
    memset(userInput_mutex.blocked_queue, 0, sizeof(userInput_mutex.blocked_queue));

    userOutput_mutex.available = true;
    userOutput_mutex.holder = NULL;
    userOutput_mutex.blocked_count = 0;
    memset(userOutput_mutex.blocked_queue, 0, sizeof(userOutput_mutex.blocked_queue));

    file_mutex.available = true;
    file_mutex.holder = NULL;
    file_mutex.blocked_count = 0;
    memset(file_mutex.blocked_queue, 0, sizeof(file_mutex.blocked_queue));

    printf("All mutexes have been reset\n");
}
int mutex_lock(mutex_t *mutex, Process *process)
{
    if (mutex == NULL || process == NULL)
    {
        fprintf(stderr, "Error: Null parameter in mutex_lock\n");
        return 1;
    }

    if (mutex->available)
    {
        mutex->available = false;
        mutex->holder = process;
        printf("Mutex %s acquired by process %d\n", mutex->name, process->pid);
        return 0;
    }
    else
    {
        if (mutex->blocked_count >= MAX_NUM_PROCESSES) {
            fprintf(stderr, "Error: Blocked queue full for mutex %s\n", mutex->name);
            return 1;
        }

        mutex->blocked_queue[mutex->blocked_count++] = process;
        process->state = BLOCKED;
        setProcessState(process->pid, BLOCKED);

        dequeue(readyQueues[getProcessPriority(process->pid)]); // Remove from ready queues
        enqueue(global_blocked_queue, process); // Add to global blocked queue

        printf("Process %d is blocked waiting for %s\n", process->pid, mutex->name);
    }
    return 1;
}

int mutex_unlock(mutex_t *mutex, Process *process)
{
    if (mutex == NULL || process == NULL)
    {
        fprintf(stderr, "Error: Null parameter in mutex_unlock\n");
        return 1;
    }

    if (!mutex->available && mutex->holder->pid == process->pid)
    {
        mutex->available = true;
        mutex->holder = NULL;
        printf("Mutex %s released by process %d\n", mutex->name, process->pid);

        if (mutex->blocked_count > 0)
        {
            int highest_pri = MAX_NUM_QUEUES;
            int selected_idx = -1;

            for (int i = 0; i < mutex->blocked_count; i++)
            {
                int current_pri = getProcessPriority(mutex->blocked_queue[i]->pid);
                if (current_pri < highest_pri)
                {
                    highest_pri = current_pri;
                    selected_idx = i;
                }
            }

            if (selected_idx != -1)
            {
                Process *next_process = mutex->blocked_queue[selected_idx];

                for (int i = selected_idx; i < mutex->blocked_count - 1; i++)
                {
                    mutex->blocked_queue[i] = mutex->blocked_queue[i + 1];
                }
                mutex->blocked_count--;

                mutex->available = false;
                mutex->holder = next_process;
                next_process->state = READY;
                setProcessState(next_process->pid, READY);

                remove_from_global_blocked_queue(next_process);

                printf("Process %d will start using %s in its coming quantum\n", next_process->pid, mutex->name);
            }
        }
        return 0;
    }
    else if (mutex->available)
    {
        fprintf(stderr, "Warning: mutex_unlock called on available mutex %s\n", mutex->name);
    }
    else
    {
        fprintf(stderr, "Error: Process %d doesn't hold mutex %s\n", process->pid, mutex->name);
    }
    return 1;
}

mutex_t *get_mutex_by_name(const char *name)
{
    if (name == NULL) return NULL;
    
    if (strcmp(name, "userInput") == 0)
        return &userInput_mutex;
    if (strcmp(name, "userOutput") == 0)
        return &userOutput_mutex;
    if (strcmp(name, "file") == 0)
        return &file_mutex;
    return NULL;
}

bool mutex_is_available(const mutex_t *mutex) {
    return mutex ? mutex->available : true;
}

const char *mutex_get_name(const mutex_t *mutex) {
    return mutex ? mutex->name : "Unknown";
}

Process *mutex_get_holder(const mutex_t *mutex) {
    return mutex ? mutex->holder : NULL;
}

int mutex_get_blocked_count(const mutex_t *mutex) {
    return mutex ? mutex->blocked_count : 0;
}

Process *mutex_get_blocked_process(const mutex_t *mutex, int index) {
    if (!mutex || index < 0 || index >= mutex->blocked_count) {
        return NULL;
    }
    return mutex->blocked_queue[index];
}

void cleanup_process_mutexes(Process *process)
{
    if (process == NULL)
        return;
    if (userInput_mutex.holder == process)
    {
        mutex_unlock(&userInput_mutex, process);
    }
    if (userOutput_mutex.holder == process)
    {
        mutex_unlock(&userOutput_mutex, process);
    }
    if (file_mutex.holder == process)
    {
        mutex_unlock(&file_mutex, process);
    }
}