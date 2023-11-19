#pragma once
#include <sys/socket.h>

struct connData
{
    int csock;
    
    struct Topic* tpcs;
    struct Client* cli;
};
