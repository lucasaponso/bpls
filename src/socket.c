#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

#include "socket.h"

int sock = 0;
pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;

int get_socket(net_config * config)
{
    printf("sock: %d\n", sock);
    struct sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    printf("sock: %d\n", sock);
    
    if (sock < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);

    if (inet_pton(AF_INET, config->ipaddress, &server_addr.sin_addr) <= 0) 
    {
        printf("Invalid address/Address not supported\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        printf("Connection failed.\n");
        close(sock);
        return 1;
    }

    return 0;
}
