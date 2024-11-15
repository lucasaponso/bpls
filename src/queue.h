#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdint.h>
// Define the size of the string data (this is an example, adjust it as necessary)
#define STRING_SIZE 64

// Define the struct for the payload data
typedef struct __attribute__((__packed__))send_payload_interval {
    char unit_number[STRING_SIZE];
    int target_payload;
    uint8_t unit_measure;
} send_payload_interval;

// Node structure for the queue
typedef struct Node {
    send_payload_interval data;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

// Function declarations for queue operations

void initQueue();                            // Initializes the global queue
int isEmpty();                               // Check if the queue is empty
void enqueue(send_payload_interval data);    // Add an element to the queue
send_payload_interval dequeue();             // Remove an element from the queue
send_payload_interval front();               // Get the front element of the queue
void freeQueue();                            // Clean up memory used by the queue

extern pthread_mutex_t queue_mutex;          // Mutex for thread safety

#endif // QUEUE_H
