#ifndef MUTEX_H
#define MUTEX_H

// typedef to be able to pass functions as parameters to mutex functions
typedef void *(*routine_func)(void *);

typedef struct {
} mutex_t;

// Locking and unlocking
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif // MUTEX_H