#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

Queue* createQueue();
void enqueue(Queue* q, int data);
int dequeue(Queue* q);
int peek(Queue* q);
int isEmpty(Queue* q);
void display(Queue* q);
void freeQueue(Queue* q);

#endif