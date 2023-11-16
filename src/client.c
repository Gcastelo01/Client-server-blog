#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "blogoperation.h"


void makeChoice(struct BlogOperation* next)
{

    char input[100];  // Ajuste o tamanho conforme necessário
    char type_action[20];
    int coords0, coords1;

    printf("Digite a ação: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    sscanf(input, "%s %d,%d", type_action, &coords0, &coords1);
    
}


int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        printf("Parâmetros: <Endereço do Servidor> <Porta do Servidor>\n");
        return 1;
    }

    char *SERVER_IP = argv[1];
    char *PORT = argv[2];

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0)
    {
        perror("Falha ao criar o soquete");
        return 1;
    }

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(PORT));

    if (inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr) <= 0)
    {
        perror("inet_pton() falhou ou endereço inválido");
        return 1;
    }

    int res = (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)));
    struct BlogOperation mov;

    if (res < 0)
    {
        perror("Connection failed");
    }
    else
    {
        mov.client_id = 0;
        mov.operation_type = 1;

        size_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
    }


    for (;;)
    {
        makeChoice(&mov);

        size_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);

        if (count != sizeof(struct BlogOperation))
        {
            perror("Falha ao enviar dados para o servidor");
            break;
        }

        size_t recv_count = recv(sock, &mov, sizeof(struct BlogOperation), 0);

        if (recv_count != sizeof(struct BlogOperation))
        {
            perror("Falha ao receber dados da resposta");
            break;
        }
        else if (recv_count == 0)
        {
            printf("resposta vazia");
        }
        else
        {
            system("clear");
            // Lógica de processamento de cada
        }
    }

    close(sock);

    return 0;
}