#pragma once

struct Client{
    int id;
    int csock;

    struct Client* nextClient;
};