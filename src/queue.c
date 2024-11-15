#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

// Declare a global queue instance
static Queue globalQueue;

// Initialize the queue (set front and rear to NULL)
void initQueue() {
    globalQueue.front = NULL;
    globalQueue.rear = NULL;
}

// Check if the queue is empty
int isEmpty() {
    return globalQueue.front == NULL;
}

// Add an element to the queue
void enqueue(tx_packet data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return;
    }
    newNode->data = data;
    newNode->next = NULL;

    if (globalQueue.rear == NULL) {
        // If the queue is empty, the new node is both the front and the rear
        globalQueue.front = globalQueue.rear = newNode;
    } else {
        // Add the new node to the end of the queue
        globalQueue.rear->next = newNode;
        globalQueue.rear = newNode;
    }
}

// Remove an element from the queue
tx_packet dequeue() 
{
    if (isEmpty()) 
    {
        printf("Queue is empty, cannot dequeue.\n");
        tx_packet empty_data = {0}; // Return an empty structure
        return empty_data;
    }

    Node* temp = globalQueue.front;
    tx_packet data = temp->data;

    // Move the front pointer to the next node
    globalQueue.front = globalQueue.front->next;

    if (globalQueue.front == NULL) 
    {
        // If the queue is now empty, set rear to NULL as well
        globalQueue.rear = NULL;
    }

    free(temp); // Free the memory of the dequeued node
    return data;
}

// Get the front element of the queue without removing it
tx_packet front() {
    if (isEmpty()) {
        printf("Queue is empty, no front element.\n");
        tx_packet empty_data = {0}; // Return an empty structure
        return empty_data;
    }
    return globalQueue.front->data;
}

// Free memory used by the queue
void freeQueue() {
    while (!isEmpty()) {
        Node* temp = globalQueue.front;
        globalQueue.front = globalQueue.front->next;
        free(temp);  // Free the dequeued node
    }
    globalQueue.rear = NULL; // Reset the rear pointer as well
}

// Mutex initialization (in case it's needed externally)
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
