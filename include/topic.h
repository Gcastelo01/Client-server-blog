#pragma once

struct Topic{
    char name_topic[50];
    int id;
    
    struct Topic* next_topic;
    int* subscribers[10];
    // lista de soquetes para implementar notificação
};