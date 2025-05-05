// Queue.h
#ifndef QUEUE_H
#define QUEUE_H
#include <stdbool.h>
#include "process.h"

typedef struct Queue
{
    Process *front;
    Process *rear;
} Queue;

Queue *createQueue();
void enqueue(Queue *q, Process *newProcess);
void enqueueWithoutClone(Queue *q, Process *process);
void enqueueSortedByArrivalTime(Queue *q, Process *newProcess);
Process *dequeue(Queue *q);
Process *peek(Queue *q);
int isEmpty(Queue *q);
int getQueueSize(Queue *q);
void displayQueue(Queue *q);
void displayQueueSimplified(Queue *q);
void freeQueue(Queue *q);
bool isQueueEmpty(Queue *q);

#endif // QUEUE_H