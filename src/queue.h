#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdint.h>

#include "rx_thread.h"

// Define the size of the string data (this is an example, adjust it as necessary)
#define STRING_SIZE 64

// Define the struct for the payload data
typedef struct __attribute__((__packed__))send_bucket_payload {
    char unit_number[STRING_SIZE];
    int target_payload;
    char unit_measure;
} send_bucket_payload;


// Define the struct for the payload data
typedef struct __attribute__((__packed__))send_final_payload {
    char final_payload[3];
    uint8_t unit_measure;
    char unit_number[STRING_SIZE];
} send_final_payload;

// Define the struct for the payload data
typedef struct __attribute__((__packed__))tx_packet {
    uint16_t msg_id;
    send_bucket_payload packet_801;
    send_final_payload packet_802;
} tx_packet;

// Node structure for the queue
typedef struct Node {
    tx_packet data;
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
void enqueue(tx_packet data);    // Add an element to the queue
tx_packet dequeue();             // Remove an element from the queue
tx_packet front();               // Get the front element of the queue
void freeQueue();                            // Clean up memory used by the queue

extern pthread_mutex_t queue_mutex;          // Mutex for thread safety

#endif // QUEUE_H
