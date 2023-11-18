#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "blogoperation.h"
#include "serverClient.h"
#include "topic.h"

int main(int argc, char *argv[])
{
    char *C_PROTOCOL = argv[1];
    in_port_t PORT = atoi(argv[2]);

    char *INPUT_m = argv[4];

    int PROTOCOLO;
    extern char *optarg;

    int servSock;

    if (strcmp(C_PROTOCOL, "v4") == 0)
    {
        PROTOCOLO = AF_INET;
    }
    else
    {
        PROTOCOLO = AF_INET6;
    }

    if ((servSock = socket(PROTOCOLO, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in servAddr;           
    memset(&servAddr, 0, sizeof(servAddr));
    
    servAddr.sin_family = PROTOCOLO;              
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servAddr.sin_port = htons(PORT);

    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("bind fail");
        return 1;
    }

    if (listen(servSock, 5) < 0)
    {
        perror("listen fail");
        return 1;
    }

    struct sockaddr_in clntAddr;
    struct BlogOperation a1, a2;

    a1.operation_type = 2;
    a1.server_response = 1;
    strcpy(a1.topic, "teste");

    a2.operation_type = 1;
    a2.server_response = 1;
    strcpy(a2.topic, "teste a2");

    socklen_t clntAddrLen = sizeof(clntAddr);

    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);

    int inutil;
    
    while(1)
    {
        printf("Tecle enter para enviar trens");
        scanf("%d", &inutil);

        size_t c1 = send(clntSock, &a1, sizeof(struct BlogOperation), 0);
        size_t c2 = send(clntSock, &a2, sizeof(struct BlogOperation), 0);
    }

    return 0;
}