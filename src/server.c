#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "action.h"

static const int MAXPENDING = 5;

int FLAGGED = 0;
int REVEALED = 0;

const int jogo_init[4][4] = {{-2, -2, -2, -2}, {-2, -2, -2, -2}, {-2, -2, -2, -2}, {-2, -2, -2, -2}};

int gabarito[4][4];


void use()
{
    printf("./server <PROTOCOL v4 || v6> <PORT> -i <INPUT FILE>");
}


int checkWin(struct action *game)
{
    int count_right = 0;

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            if(game->board[i][j] == -3 && gabarito[i][j] == -1){
                count_right++;
            }
        }
    }

    if(count_right == 3){
        return 1;
    }
    
    return 0;
}


void printGame(struct action *mov)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int valor = mov->board[i][j];

            if (valor == -2)
            {
                printf("- ");
            }
            else if (valor == 0)
            {
                printf("0 ");
            }
            else if (valor == -3)
            {
               printf("> ");
            }
            else if (valor == -1)
            {
                printf("* ");
            }
            
            else
            {
                printf("%d ", valor);
            }
        }
        printf("\n");
    }
}


void geraJogo(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (fscanf(file, "%d,", &(gabarito)[i][j]) != 1)
            {
                perror("Erro ao ler o arquivo");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}


void fazJogada(struct action *acao)
{   
    if(acao->type == 0)
    {
        acao->type = 3;

        memcpy(&(acao->board), &jogo_init, sizeof(int) * 4 * 4);
    } 
    else if (acao->type == 1)
    {
        int coord0 = acao->coordinates[0];
        int coord1 = acao->coordinates[1];

        int valor = (gabarito)[coord0][coord1];

        
        if (acao->board[coord0][coord1] == -3)
        {
            printf("error: cell already flaged");
        }
        else if (valor == -1)
        {
            acao->type = 8;
            
            for(int i = 0; i < 4; i++){
                for(int j = 0; j < 4; j++){
                    acao->board[i][j] = gabarito[i][j];
                }
            }
        }
        else
        {
            acao->type = 3;
            
            (acao->board)[coord0][coord1] = valor;

            REVEALED++;
            
            if(REVEALED == 13)
            {
                acao->type = 6;
                
                for(int i = 0; i < 4; i++){
                    for(int j = 0; j < 4; j++){
                        acao->board[i][j] = gabarito[i][j];
                    }
                }
            }
        }
    }
    else if (acao->type == 2)
    {
        acao->type = 3;

        int coord0 = acao->coordinates[0];
        int coord1 = acao->coordinates[1];  

        if (acao->board[coord0][coord1] == -3)
        {
            printf("Error: cell already flaged\n");
        }
        else if(acao->board[coord0][coord1] != -2)
        {
            printf("Error: cell already revealed");    
        }
        else
        {
            acao->board[coord0][coord1] = -3;
            FLAGGED++;

            if(FLAGGED == 3){
                if(checkWin(acao)){
                    acao->type = 6;

                    for(int i = 0; i < 4; i++){
                        for(int j = 0; j < 4; j++){
                            acao->board[i][j] = gabarito[i][j];
                        }
                    }
                }
            }
        }
    }
    else if(acao->type == 4)
    {
        int coord0 = acao->coordinates[0];
        int coord1 = acao->coordinates[1];

        if (acao->board[coord0][coord1] != -3)
        {
            printf("error: cell not flaged\n");
        }
        else
        {
            acao->board[coord0][coord1] = -2;
            FLAGGED--;
        }   
    }
    else if(acao->type == 5)
    {
        acao->type = 3;
        FLAGGED = 0;
        REVEALED = 0;

        memcpy(&(acao->board), &jogo_init, sizeof(int) * 4 * 4);
    }
}


int main(int argc, char *argv[])
{
    char *C_PROTOCOL = argv[1];
    in_port_t PORT = atoi(argv[2]);

    char *INPUT_m = argv[4];

    int PROTOCOLO;
    extern char *optarg;

    int servSock;

    geraJogo(INPUT_m);

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
    struct action mov;

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            mov.board[i][j] = -2;
        }
    }

    for (;;)
    {

        socklen_t clntAddrLen = sizeof(clntAddr);

        int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);

        while (1)
        {
            size_t numbytesrec = recv(clntSock, &mov, sizeof(struct action), 0);

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
                fazJogada(&mov);
                
                send(clntSock, &mov, sizeof(struct action), 0);
            }
        }
    }

    return 0;
}