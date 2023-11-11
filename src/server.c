#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "blogoperation.h"

static const int MAXPENDING = 5;

int FLAGGED = 0;
int REVEALED = 0;


void use()
{
    printf("./server <PROTOCOL v4 || v6> <PORT> -i <INPUT FILE>");
}


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


    struct sockaddr_in servAddr;                  // Local address
    memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
    
    
    servAddr.sin_family = PROTOCOLO;              // Corrigido o tipo de protocolo
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    servAddr.sin_port = htons(PORT);

    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("bind fail");
        return 1;
    }

    if (listen(servSock, MAXPENDING) < 0)
    {
        perror("listen fail");
        return 1;
    }

    struct sockaddr_in clntAddr;
    struct BlogOperation mov;


    for (;;)
    {

        socklen_t clntAddrLen = sizeof(clntAddr);

        int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);

        while (1)
        {
            size_t numbytesrec = recv(clntSock, &mov, sizeof(struct BlogOperation), 0);

            if (numbytesrec < 0)
            {
                perror("Erro na recepção");
                break;
            }
            else if (numbytesrec == 0)
            {
                printf("Cliente encerrou a conexão\n");
                break;
            }
            else
            {
                // Lógica de processamento dos clientes
            }
        }
    }

    return 0;
}