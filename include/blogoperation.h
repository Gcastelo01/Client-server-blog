#pragma once

struct BlogOperation{
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};