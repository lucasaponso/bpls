#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "rx_thread.h"
#include "pay_load_adv.h"
#include "queue.h"
#include "socket.h"
#include "crc.h"

#define PREAMBLE 0x7C
#define TCP_MESSAGE_SIZE 1500
#define BUFFER_SIZE 1500

#define MAX_STRINGS 12

typedef enum {
    IDLE,
    LENGTH_LOW,
    LENGTH_HIGH,
    DATA
} rx_thread_state;

void process_packet(uint8_t output_buffer[TCP_MESSAGE_SIZE], int index);
truck_begin_loading process_0801(uint8_t output_buffer[TCP_MESSAGE_SIZE], int len);
char ** validate_data_buffer(uint8_t data_buffer[TCP_MESSAGE_SIZE], int len);
truck_end_loading process_0802(uint8_t output_buffer[TCP_MESSAGE_SIZE], int len);

void* rx_thread_init(void* arg) 
{    
    rx_thread_state state = IDLE;
    int bytes_rec = 0;

    uint8_t buffer[BUFFER_SIZE];
    uint8_t output_buffer[TCP_MESSAGE_SIZE];
    int index = 0;

    uint8_t length_low = 0, length_high = 0;
    u_int16_t msg_length = 0;
    
    pthread_mutex_lock(&sock_mutex);
    
    while ((bytes_rec = read(sock, buffer, sizeof(buffer))) > 0)
    {   
        printf("processing packet...\n");

        for (int i = 0; i < bytes_rec; i++)
        {
            switch (state)
            {
                case IDLE:
                    if (buffer[i] == 0x7c)
                    {
                        output_buffer[index++] = buffer[i];
                        state = LENGTH_LOW;
                    }                    
                    else
                    {
                        index = 0;
                    }
                    break;
                case LENGTH_LOW:
                    output_buffer[index++] = buffer[i];
                    length_low =  buffer[i];
                    state = LENGTH_HIGH;
                    break;
                case LENGTH_HIGH:
                    output_buffer[index++] = buffer[i];
                    length_high =  buffer[i];

                    msg_length = length_high;
                    msg_length = (msg_length << 8) & 0xff00;
                    msg_length |= (u_int16_t)length_low;
                    msg_length += 1; //this works? 
                    state = DATA;
                    break;
                case DATA:
                    output_buffer[index++] = buffer[i];
                    if (msg_length > 0)
                    {
                        msg_length--;
                    }
                    else if (msg_length == 0)
                    {               
                        for (int i = 0; i < index; i++)
                        {
                            printf("%02x ", output_buffer[i]);
                        }
                        printf("\n");

                        if (validate_crc(output_buffer, index) == 0)
                        {
                            printf("crc was validated correctly.\n");
                            process_packet(output_buffer, index);
                        }
                        
                        state = IDLE;
                    }
                    break;
                default:
                    break;
                }
        }
        index = 0;
        printf("end processing packet\n");
        
        printf("\n");
    }
    pthread_mutex_unlock(&sock_mutex);
    
    return NULL;
}

void process_packet(uint8_t output_buffer[TCP_MESSAGE_SIZE], int len)
{
    int index = 5;
    uint8_t data_buffer[TCP_MESSAGE_SIZE];
    uint16_t msg_id = *(uint16_t *)&output_buffer[index]; //retrieve msg_id from buffer
    msg_id = ntohs(msg_id);

    printf("processing %04x msg\n", msg_id);
    
    switch (msg_id)
    {
        tx_packet p_tx;
        
        case TRUCK_BEGIN_LOADING:
        printf("ok\n");
            p_tx.msg_id = msg_id;
            printf("ok\n");
            truck_begin_loading packet = process_0801(&output_buffer[index + 2], len - 2);
            send_bucket_payload p_interval;

            strncpy(p_interval.unit_number, packet.unit_number, STRING_SIZE - 1); //copy
            p_interval.unit_number[STRING_SIZE - 1] = '\0';  // Ensure null termination
            
            p_interval.unit_measure = 'M';
            
            int target_payload = atoi(packet.target_payload);
            p_interval.target_payload = target_payload / 5;

            printf("ok\n");
            p_tx.packet_801 = p_interval;

            pthread_mutex_lock(&queue_mutex);
            enqueue(p_tx);
            pthread_mutex_unlock(&queue_mutex);
            break;
        case TRUCK_END_LOADING:
            printf("msg_id to be implemented...\n");
            p_tx.msg_id = msg_id;
            truck_end_loading p_end = process_0802(&output_buffer[index + 2], len - 2);
            send_final_payload p_final;

            strncpy(p_final.unit_number, p_end.unit_number, STRING_SIZE - 1);
            p_final.unit_number[STRING_SIZE - 1] = '\0';

            p_final.unit_measure = 'M';

            // memcpy(p_final.final_payload, &final_payload, sizeof(final_payload));  // Copy contents

            p_tx.packet_802 = p_final;
            
            // pthread_mutex_lock(&queue_mutex);
            // enqueue(p_tx);
            // pthread_mutex_unlock(&queue_mutex);

            break;
        case TRUCK_PING_MSG:
            printf("ping msg recieved\n");
            ping_asm_packet();
            break;
        default:
            printf("msg_id not implemented: %04x\n", msg_id);
            break;
    }
}

truck_begin_loading process_0801(uint8_t output_buffer[TCP_MESSAGE_SIZE], int len)
{
    
    truck_begin_loading packet;
    char ** strings = validate_data_buffer(output_buffer, len);
    
    strncpy(packet.unit_number, strings[0], STRING_SIZE - 1);
    strncpy(packet.target_payload, strings[1], 3);  // 3 bytes for target_payload
    strncpy(packet.fleet_identifier, strings[2], STRING_SIZE - 1);
    strncpy(packet.latitude, strings[3], STRING_SIZE - 1);
    strncpy(packet.longitude, strings[4], STRING_SIZE - 1);
    strncpy(packet.altitude, strings[5], STRING_SIZE - 1);
    strncpy(packet.timestamp, strings[6], STRING_SIZE - 1);
    strncpy(packet.material, strings[7], STRING_SIZE - 1);
    strncpy(packet.shovel_operator, strings[8], STRING_SIZE - 1);
    strncpy(packet.easting, strings[9], STRING_SIZE - 1);
    strncpy(packet.northing, strings[10], STRING_SIZE - 1);
    strncpy(packet.elevation, strings[11], STRING_SIZE - 1);

    // Print the values to verify it worked
    printf("Unit Number: %s\n", packet.unit_number);
    printf("Target Payload: %s\n", packet.target_payload);
    printf("Fleet Identifier: %s\n", packet.fleet_identifier);
    printf("Latitude: %s\n", packet.latitude);
    printf("Longitude: %s\n", packet.longitude);
    printf("Altitude: %s\n", packet.altitude);
    printf("Timestamp: %s\n", packet.timestamp);
    printf("Material: %s\n", packet.material);
    printf("Shovel Operator: %s\n", packet.shovel_operator);
    printf("Easting: %s\n", packet.easting);
    printf("Northing: %s\n", packet.northing);
    printf("Elevation: %s\n", packet.elevation);

    return packet;
}

truck_end_loading process_0802(uint8_t output_buffer[TCP_MESSAGE_SIZE], int len)
{
    
    truck_end_loading packet;
    char ** strings = validate_data_buffer(output_buffer, len);

    strncpy(packet.unit_number, strings[0], STRING_SIZE - 1);
    strncpy(packet.payload, strings[1], 3);  // 3 bytes for target_payload

    printf("Unit Number: %s\n", packet.unit_number);
    printf("Payload: %s\n", packet.payload);
    
    return packet;
}


char ** validate_data_buffer(uint8_t data_buffer[TCP_MESSAGE_SIZE], int len)
{
    int i = 0;
    int buffer_index = 0;  // To track position in current string
    char** strings = (char**)malloc(MAX_STRINGS * sizeof(char*));  // Allocate memory for string array
    char current_string[TCP_MESSAGE_SIZE];  // Temporary buffer to store each string
    int string_count = 0;  // Number of strings found

    // Step 1: Process the input buffer
    while (i < len) 
    {
        // If we find a null terminator, we process the current string
        if (data_buffer[i] == '\0') 
        {
            // Null terminator found, save the current string and reset the buffer
            current_string[buffer_index] = '\0';  // Null-terminate the string
            if (string_count < MAX_STRINGS) 
            {
                strings[string_count++] = strdup(current_string);  // Save the string
                // printf("String %d: %s\n", string_count, current_string);  // Print the string

                // If it's the first string, read the next 3 bytes and store them as a string
                if (string_count == 1) 
                {
                    i++;  // Move past the null terminator
                    if (i + 3 <= len)  // Ensure we have at least 3 more bytes
                    {
                        // Convert the next 3 bytes into a string
                        char next_three_bytes[4];  // 3 bytes + null terminator
                        
                        // Store the next 3 bytes as characters (not hex)
                        next_three_bytes[0] = (char)data_buffer[i];
                        next_three_bytes[1] = (char)data_buffer[i+1];
                        next_three_bytes[2] = (char)data_buffer[i+2];
                        next_three_bytes[3] = '\0';  // Null-terminate the string

                        // Store this new string in the array
                        strings[string_count++] = strdup(next_three_bytes);
                        // printf("Next 3 bytes as string: %s\n", next_three_bytes);

                        i += 3;  // Skip over the 3 bytes we've just read
                    }
                    else 
                    {
                        // printf("Not enough bytes after null terminator for 3-byte read\n");
                        break;  // Exit if we don't have enough bytes
                    }
                }
            }
            else 
            {
                // printf("Maximum number of strings reached.\n");
                break;
            }
            buffer_index = 0;  // Reset the buffer index for the next string
        } 
        else 
        {
            // Copy the current byte into the buffer
            if (buffer_index < TCP_MESSAGE_SIZE - 1)  // Ensure we don't overflow the buffer
            {
                current_string[buffer_index++] = (char)data_buffer[i];
            }
            else
            {
                // printf("Buffer overflow while processing string\n");
                break;
            }
        }

        i++;  // Move to the next byte in the input buffer
    }

    return strings;
}