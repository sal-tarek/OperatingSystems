// Queue.h
#ifndef QUEUE_H
#define QUEUE_H
#include "PCB.h"

typedef struct Queue {
    Process* front;
    Process* rear;
} Queue;

Queue* createQueue();
void enqueue(Queue* q, Process* newProcess);
Process* dequeue(Queue* q);
Process* peek(Queue* q);
int isEmpty(Queue* q);
void display(Queue* q);
void freeQueue(Queue* q);

#endif // QUEUE_H