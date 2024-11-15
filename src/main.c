#include <stdio.h>
#include <pthread.h>

#include "rx_thread.h"
#include "pay_load_adv.h"
#include "queue.h"
#include "socket.h"
#include "crc.h"

/*
 * main - Main entry point for the client program that connects to a remote server
 *        and spawns threads for receiving data and sending payload advertisements.
 *
 * This program connects to a remote server using a TCP socket, either using default
 * parameters or those specified on the command line. It spawns two threads:
 * 1. A receive thread (`rx_thread`) that listens for incoming data from the server.
 * 2. A payload advertisement thread (`pay_load_adv`) that periodically sends payload
 *    data to the server.
 * 
 * Parameters:
 *   int argc   - The number of command-line arguments.
 *   char **argv - The command-line arguments.
 * 
 * Usage:
 *   - Default usage: `./program`
 *   - Usage with command-line parameters: `./program <ip_address> <port>`
 *
 * The program will:
 * - Parse command-line arguments for the server's IP address and port.
 * - Establish a connection to the specified server using the TCP protocol.
 * - Spawn two threads: one for receiving data and one for sending periodic payload advertisements.
 * 
 * Notes:
 *   - If no command-line parameters are provided, the default IP address (127.0.0.1)
 *     and port (12345) are used.
 *   - If more than two arguments are provided, the program will print an error message and exit.
 *   - If the port number is invalid (not in the range 1-65535), the program will print an error message and exit.
 *   - If socket creation or thread creation fails, the program will print an error message and terminate.
 */
int main(int argc, char **argv)
{
    net_config * config;
    config->ipaddress = "127.0.0.1";
    config->port = 12345;
    
    if (argc == 3) 
    {
        config->ipaddress = argv[1];
        config->port = atoi(argv[2]);
        
        if (config->port <= 0 || config->port > 65535) 
        {
            printf("invalid port number. it must be between 1 and 65535.\n");
            return 1;
        }
    } 
    else if (argc > 3) 
    {
        printf("usage: %s [ip_address] [port]\n", argv[0]);
        return 1;
    }

    printf("connecting to IP: %s on port: %d\n", config->ipaddress, config->port);

    if (get_socket(config) != 0)
    {
        printf("socket crashed\n");
    }
    
    initQueue();

    pthread_t rx_thread;
    
    if (pthread_create(&rx_thread, NULL, rx_thread_init, NULL) != 0)
    {
        printf("error spawning rx_thread \n");
        return -1;
    }

    printf("spawning rx_thread.\n");
    
    
    pthread_t pay_load_adv;

    if (pthread_create(&pay_load_adv, NULL, pay_load_adv_thread_init, NULL) != 0)
    {
        printf("error spawning payload advertisement thread\n");
        return -1;
    }

    printf("spawning payload_adv thread.\n");

    pthread_join(rx_thread, NULL);
    pthread_join(pay_load_adv, NULL);
    
    return 0;
}