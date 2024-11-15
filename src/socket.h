#ifndef SOCKET_H
#define SOCKET_H

#include <pthread.h>

typedef struct {
    char *ipaddress;
    int port;
} net_config;

int get_socket(net_config * config);

extern int sock;
extern pthread_mutex_t sock_mutex;


#endif //SOCKET_H