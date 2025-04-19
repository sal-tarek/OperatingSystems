#include <stdio.h>
#include <stdlib.h>
#include "PCB.h"

typedef struct Queue {
    Process* front;
    Process* rear;
} Queue;

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, Process* newProcess) {
    newProcess->state = READY;

    if (q->rear == NULL) {
        q->front = q->rear = newProcess;
        return;
    }

    q->rear->next = newProcess;
    q->rear = newProcess;
}

Process* dequeue(Queue* q) {
    if (q->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }

    Process* temp = q->front;  
    q->front = q->front->next;  

    if (q->front == NULL) {
        q->rear = NULL;  
    }

    temp->next = NULL;  
    return temp;  
}

Process* peek(Queue* q) {
    if (q->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }
    return q->front;
}

int isEmpty(Queue* q) {
    return (q->front == NULL);
}

void display(Queue* q) {
    Process* curr = q->front;
    printf("Queue: ");
    while (curr != NULL) {
        printf("%d -> ", curr->id);
        curr = curr->next;
    }
    printf("NULL\n");
}


void freeQueue(Queue* q) {
    Process* curr = q->front;
    Process* next;
    while (curr != NULL) {
        next = curr->next;
        free(curr->file_path); // Free file_path
        free(curr);            // Free Process struct
        curr = next;
    }
    free(q);
}