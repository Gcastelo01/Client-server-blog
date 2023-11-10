#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "action.h"


void printGame(struct action *mov)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int valor = mov->board[i][j];

            if (valor == -2)
            {
                printf("-\t\t");
            }
            else if (valor == 0)
            {
                printf("0\t\t");
            }
            else if (valor == -3)
            {
               printf(">\t\t");
            }
            else if (valor == -1)
            {
                printf("*\t\t");
            }
            else
            {
                printf("%d\t\t", valor);
            }
        }
        printf("\n");
    }
}


void makeChoice(struct action* next)
{

    char input[100];  // Ajuste o tamanho conforme necessário
    char type_action[20];
    int coords0, coords1;

    printf("Digite a ação: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    sscanf(input, "%s %d,%d", type_action, &coords0, &coords1);

    if (strcmp(type_action, "start") == 0)
    {
        next->type = 0;

    }
    else if (strcmp(type_action, "reveal") == 0 || strcmp(type_action, "flag") == 0 ||
             strcmp(type_action, "remove_flag") == 0)
    {
        next->type = (strcmp(type_action, "reveal") == 0) ? 1 : ((strcmp(type_action, "flag") == 0) ? 2 : 4);
        next->coordinates[0] = coords0;
        next->coordinates[1] = coords1;
    }
    else if (strcmp(type_action, "reset") == 0)
    {
        next->type = 5;
    }
    else if (strcmp(type_action, "exit") == 0)
    {
        next->type = 7;
    }
    else
    {
        printf("Ação inválida. Tente novamente.\n");
        next->type = -1; 
    }
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

    if (res < 0)
    {
        perror("Connection failed");
    }

    struct action mov;

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            mov.board[i][j] = -2;
        }
    }

    for (;;)
    {
        makeChoice(&mov);

        size_t count = send(sock, &mov, sizeof(struct action), 0);

        if (count != sizeof(struct action))
        {
            perror("Falha ao enviar dados para o servidor");
            break;
        }

        size_t recv_count = recv(sock, &mov, sizeof(struct action), 0);

        if (recv_count != sizeof(struct action))
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
            
            if (mov.type == 7)
            {
                close(sock);
                return 0;
            }
            else if(mov.type == 6)
            {
                printf("YOU WIN!\n");
                printGame(&mov);
            }
            else if(mov.type == 8)
            {
                printf("GAME OVER!\n");
                printGame(&mov);

            }
            else
            {
                printGame(&mov);
            }
        }
    }

    close(sock);

    return 0;
}