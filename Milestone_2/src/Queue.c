#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "process.h"
#include "Queue.h"

Queue *createQueue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue *q, Process *process)
{
    // Ensure the process has no lingering next pointer
    process->next = NULL;

    // Add the process directly to the queue without cloning
    if (q->rear == NULL)
    {
        q->front = q->rear = process;
    }
    else
    {
        q->rear->next = process;
        q->rear = process;
    }
}

void enqueueSortedByArrivalTime(Queue *q, Process *newProcess)
{
    newProcess->next = NULL;

    // Case 1: Queue is empty or newProcess should be placed at the front
    if (q->front == NULL || newProcess->arrival_time < q->front->arrival_time)
    {
        newProcess->next = q->front;
        q->front = newProcess;
        if (q->rear == NULL)
        {
            q->rear = newProcess;
        }
        return;
    }

    // Case 2: Insert in the correct sorted position
    Process *current = q->front;
    while (current->next != NULL && current->next->arrival_time <= newProcess->arrival_time)
    {
        current = current->next;
    }

    newProcess->next = current->next;
    current->next = newProcess;

    // If inserted at the end, update rear
    if (newProcess->next == NULL)
    {
        q->rear = newProcess;
    }
}

Process *dequeue(Queue *q)
{
    if (q->front == NULL)
    {
        printf("Can't dequeue, Queue is empty\n");
        return NULL;
    }

    Process *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL)
    {
        q->rear = NULL;
    }

    temp->next = NULL;
    return temp;
}

Process *peek(Queue *q)
{
    if (q->front == NULL)
    {
        printf("No peek, Queue is empty\n");
        return NULL;
    }
    return q->front;
}

int isEmpty(Queue *q)
{
    return (q->front == NULL);
}

int getQueueSize(Queue *q)
{
    int size = 0;
    Process *current = q->front;
    while (current != NULL)
    {
        size++;
        current = current->next;
    }
    return size;
}

void displayQueue(Queue *q)
{
    Process *curr = q->front;
    printf("Queue:\n");
    while (curr != NULL)
    {
        displayProcess(curr); // Print details of each process
        curr = curr->next;
    }
    printf("End of Queue\n");
}

void displayQueueSimplified(Queue *q)
{
    Process *curr = q->front;
    printf("Queue: ");
    while (curr != NULL)
    {
        printf("%d -> ", curr->pid);
        curr = curr->next;
    }
    printf("NULL\n");
}

void freeQueue(Queue *q)
{
    Process *curr = q->front;
    Process *next;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr->file_path); // Free file_path
        free(curr);            // Free Process struct
        curr = next;
    }
    free(q);
}

bool isQueueEmpty(Queue *q)
{
    return q == NULL || q->front == NULL;
}
