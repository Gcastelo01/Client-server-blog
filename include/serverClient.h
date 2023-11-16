#pragma once

struct Client{
    int id;

    struct Client* nextClient;
};