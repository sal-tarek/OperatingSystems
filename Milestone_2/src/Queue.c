#include <stdio.h>
#include <stdlib.h>

// Node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, int data) {
    Node* newNode = createNode(data);
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

int dequeue(Queue* q) {
    if (q->front == NULL) {
        printf("Queue is empty\n");
        return -1;
    }

    int data = q->front->data;
    Node* temp = q->front->next;
    q->front->next = NULL;
    q->front = temp;


    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
    return data;
}

int peek(Queue* q) {
    if (q->front == NULL) {
        printf("Queue is empty\n");
        return -1;
    }
    return q->front->data;
}

int isEmpty(Queue* q) {
    return (q->front == NULL);
}

void display(Queue* q) {
    Node* curr = q->front;
    printf("Queue: ");
    while (curr != NULL) {
        printf("%d -> ", curr->data);
        curr = curr->next;
    }
    printf("NULL\n");
}

int main() {
    Queue* q = createQueue();

    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);
    display(q);

    dequeue(q);
    display(q);
    dequeue(q);
    display(q);
    dequeue(q);
    display(q);

    return 0;
}