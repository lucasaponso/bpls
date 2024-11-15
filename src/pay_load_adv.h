#ifndef PAY_LOAD_ADV_H
#define PAY_LOAD_ADV_H

#include <pthread.h>
#include <stdio.h>


// Function declaration for the thread function
void* pay_load_adv_thread_init(void* arg);
void ping_asm_packet();

extern uint16_t sequence_id;
extern int final_payload;

// Optional: Additional functions or data structures can be declared here

#endif // PAY_LOAD_ADV_H
