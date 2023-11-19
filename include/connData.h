#pragma once
#include <sys/socket.h>
#include <pthread.h>

struct connData
{
    int csock;
    
    struct Topic* tpcs;
    struct Client* cli;
    pthread_mutex_t *mutex;
};
