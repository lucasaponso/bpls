
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

#define PREAMBLE_LEN 1
#define MSG_LEN 2
#define SEQ_ID_LEN 2
#define MSG_ID_LEN 2
#define CRC_LEN 2

uint16_t sequence_id = 0;
int final_payload = 0;

void pay_load_adv_send(send_bucket_payload data);
void* pay_load_adv_thread_init(void* arg); 
void pay_load_asm_packet(send_bucket_payload data);
int send_packet(uint8_t *send_arr, int length);
void pay_load_asm_packet_0802(send_final_payload data);

void* pay_load_adv_thread_init(void* arg) 
{    
    tx_packet data;

    while (1)
    {
        pthread_mutex_lock(&queue_mutex);

        if (!isEmpty()) 
        {
            data = dequeue();

            if (data.msg_id == TRUCK_BEGIN_LOADING)
            {
                for (int i = 0; i < 5; i++)
                {
                    pay_load_asm_packet(data.packet_801);
                    data.packet_801.target_payload++;
                    sleep(30);
                }
                
            }
            else if (data.msg_id == TRUCK_END_LOADING)
            {
                pay_load_asm_packet_0802(data.packet_802);
            }
            
            freeQueue();
        }

        pthread_mutex_unlock(&queue_mutex);
        sleep(1);
    }

    return NULL;
}

void pay_load_asm_packet(send_bucket_payload data)
{
    int index = 0; //incrementing counter in output array
    uint16_t msg_id = 0x0901;
    uint8_t target_payload_bytes[3];
    char formatted_str[4];

    uint16_t msg_length = SEQ_ID_LEN + MSG_ID_LEN + 3 + 1 + strlen(data.unit_number) + 1; 
    uint8_t send_arr[msg_length + PREAMBLE_LEN + MSG_LEN + CRC_LEN];

    memset(send_arr, 0, sizeof(send_arr));

    //Assemble Packet
    send_arr[index++] = 0x7C;
    send_arr[index++] = (uint8_t)(msg_length & 0xFF);  // Low byte msg_len
    send_arr[index++] = (uint8_t)((msg_length >> 8) & 0xFF);  // High byte msg_len
    send_arr[index++] = (uint8_t)((sequence_id >> 8) & 0xFF); //low byte seq_id
    send_arr[index++] = (uint8_t)(sequence_id & 0xFF);  //high byte seq_id
    send_arr[index++] = (uint8_t)((msg_id >> 8) & 0xFF);  // High byte of Message ID
    send_arr[index++] = (uint8_t)(msg_id & 0xFF);  // Low byte of Message ID

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

        if (send_packet(send_arr, index) != -1)
        {
            sequence_id++;
            final_payload += data.target_payload;
        }
        else
        {
            printf("unable to send packet\n");
        }
}



void pay_load_asm_packet_0802(send_final_payload data)
{
    int index = 0; //incrementing counter in output array
    uint16_t msg_id = 0x0902;
    uint8_t target_payload_bytes[3];
    char formatted_str[4];
    uint16_t msg_length = SEQ_ID_LEN + MSG_ID_LEN + 3 + 1 + strlen(data.unit_number) + 1; 
    uint8_t send_arr[msg_length + PREAMBLE_LEN + MSG_LEN + CRC_LEN];

    memset(send_arr, 0, sizeof(send_arr));

    
    //Assemble Packet
    send_arr[index++] = 0x7C;
    send_arr[index++] = (uint8_t)(msg_length & 0xFF);  // Low byte msg_len
    send_arr[index++] = (uint8_t)((msg_length >> 8) & 0xFF);  // High byte msg_len
    send_arr[index++] = (uint8_t)((sequence_id >> 8) & 0xFF); //low byte seq_id
    send_arr[index++] = (uint8_t)(sequence_id & 0xFF); //high byte seq_id
    send_arr[index++] = (uint8_t)((msg_id >> 8) & 0xFF);  // High byte of Message ID
    send_arr[index++] = (uint8_t)(msg_id & 0xFF);  // Low byte of Message ID


    snprintf(formatted_str, sizeof(formatted_str), "%03d", data.final_payload);
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
            break;
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


    if (send_packet(send_arr, index) != -1)
    {
        sequence_id++;
        final_payload = 0;
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

    if (send_packet(send_arr, index) != -1)
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
