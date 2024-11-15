
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"
#include "socket.h"
#include "crc.h"
#include "pay_load_adv.h"

uint16_t sequence_id = 0;
int final_payload = 0;

void pay_load_adv_send(send_payload_interval data);
void* pay_load_adv_thread_init(void* arg); 
void pay_load_asm_packet(send_payload_interval data);
int send_packet(uint8_t *send_arr, int length);

void* pay_load_adv_thread_init(void* arg) 
{    
    int *thread_id = (int*) arg;
    send_payload_interval data;

    while (1)
    {
        pthread_mutex_lock(&queue_mutex);

        if (!isEmpty()) 
        {
            data = dequeue();

            for (int i = 0; i < 5; i++)
            {
                pay_load_asm_packet(data);
                data.target_payload++;
                sleep(30);
            }
            
            freeQueue();
        }

        pthread_mutex_unlock(&queue_mutex);
        sleep(1);
    }

    return NULL;
}

void pay_load_asm_packet(send_payload_interval data)
{
    int index = 0; //incrementing counter in output array
    uint16_t msg_id = 0x0901;

    uint16_t msg_length = 2 + 2 + 3 + 1 + strlen(data.unit_number) + 1; 
    

    uint8_t send_arr[msg_length + 3 + 2];
    memset(send_arr, 0, sizeof(send_arr));

    send_arr[index++] = 0x7C;

    send_arr[index++] = (uint8_t)(msg_length & 0xFF);  // Low byte
    send_arr[index++] = (uint8_t)((msg_length >> 8) & 0xFF);  // High byte

    send_arr[index++] = (uint8_t)((sequence_id >> 8) & 0xFF);
    send_arr[index++] = (uint8_t)(sequence_id & 0xFF);

    send_arr[index++] = (uint8_t)((msg_id >> 8) & 0xFF);  // High byte of Message ID
    send_arr[index++] = (uint8_t)(msg_id & 0xFF);  // Low byte of Message ID

    uint8_t target_payload_bytes[3];
    char formatted_str[4];

    snprintf(formatted_str, sizeof(formatted_str), "%03d", data.target_payload);
    memcpy(target_payload_bytes, formatted_str, 3);

    for (int i = 0; i < 3; i++) 
    {
        send_arr[index++] = target_payload_bytes[i];
    }

    send_arr[index++] = (uint8_t)data.unit_measure;

    for (int i = 0; i < sizeof(data.unit_number); i++) 
    {
        send_arr[index++] = (uint8_t)data.unit_number[i];

        if (data.unit_number[i] == '\0') 
        {
            break;  // Stop copying at null terminator
        }
    }

    uint16_t checksum = slipcrc(send_arr, index);

    send_arr[index++] = (uint8_t)(checksum & 0xFF);
    send_arr[index++] = (uint8_t)((checksum >> 8) & 0xFF);

    for (int i = 0; i < index; i++) 
    {
        printf("0x%02X ", send_arr[i]);
    }
    printf("\n");

    
    if (send_packet(send_arr, index) == 0)
    {
        sequence_id++;
        final_payload += data.target_payload;
    }
    else
    {
        printf("unable to send packet\n");
    }
}


void ping_asm_packet()
{
    int index = 0; //incrementing counter in output array
    uint16_t msg_id = 0x0903;

    uint16_t msg_length = 2 + 2; 
    

    uint8_t send_arr[msg_length + 2 + 2 + 1];
    memset(send_arr, 0, sizeof(send_arr));

    send_arr[index++] = 0x7C;

    send_arr[index++] = (uint8_t)(msg_length & 0xFF);  // Low byte
    send_arr[index++] = (uint8_t)((msg_length >> 8) & 0xFF);  // High byte

    send_arr[index++] = (uint8_t)((sequence_id >> 8) & 0xFF);
    send_arr[index++] = (uint8_t)(sequence_id & 0xFF);

    send_arr[index++] = (uint8_t)((msg_id >> 8) & 0xFF);  // High byte of Message ID
    send_arr[index++] = (uint8_t)(msg_id & 0xFF);  // Low byte of Message ID

    uint16_t checksum = slipcrc(send_arr, index);

    send_arr[index++] = (uint8_t)(checksum & 0xFF);
    send_arr[index++] = (uint8_t)((checksum >> 8) & 0xFF);

    for (int i = 0; i < index; i++) 
    {
        printf("0x%02X ", send_arr[i]);
    }
    printf("\n");

    if (send_packet(send_arr, index) == 0)
    {
        sequence_id++;
    }
    else
    {
        printf("unable to send packet\n");
    }
}

int send_packet(uint8_t *send_arr, int length)
{
    ssize_t bytes_sent = send(sock, send_arr, length, 0);
    return bytes_sent;
}
