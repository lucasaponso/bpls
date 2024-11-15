#ifndef RX_THREAD_H
#define RX_THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define STRING_SIZE 64

#define TRUCK_BEGIN_LOADING 0x0801
#define TRUCK_END_LOADING 0x0802
#define TRUCK_PING_MSG 0x0803

typedef struct __attribute__((__packed__)) truck_begin_loading { // MessageID 0x08 01
    char unit_number[STRING_SIZE];      // Null-terminated string for unit number (e.g., "T151")
    char target_payload[4];             // 3 characters + 1 for null-termination (e.g., "131")
    char fleet_identifier[STRING_SIZE]; // Null-terminated string for fleet identifier (e.g., "Fleet 1")
    char latitude[STRING_SIZE];         // Null-terminated string for latitude (e.g., "49.18588")
    char longitude[STRING_SIZE];        // Null-terminated string for longitude (e.g., "-123.10615")
    char altitude[STRING_SIZE];         // Null-terminated string for altitude (e.g., "-172")
    char timestamp[STRING_SIZE];        // Null-terminated string for timestamp (e.g., "10/16/2012 06:42:23PM")
    char material[STRING_SIZE];         // Null-terminated string for material (e.g., "M01")
    char shovel_operator[STRING_SIZE];  // Null-terminated string for shovel operator (e.g., "Finch, Jack")
    char easting[STRING_SIZE];          // Null-terminated string for easting (e.g., "1052.02")
    char northing[STRING_SIZE];         // Null-terminated string for northing (e.g., "6653.12")
    char elevation[STRING_SIZE];        // Null-terminated string for elevation (e.g., "103.50")
} truck_begin_loading;


// Function declaration for the thread function
void* rx_thread_init(void* arg);

// Optional: Additional functions or data structures can be declared here

#endif // RX_THREAD_H
