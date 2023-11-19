#pragma once
#include <sys/socket.h>
#include <pthread.h>

struct connData
{
    int csock;
    
    pthread_mutex_t *mutex;
};
