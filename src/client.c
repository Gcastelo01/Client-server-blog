#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "blogoperation.h"

int id = 0;

/**
 * @brief Molda a operação a ser transmitida para o servidor de acordo com o input digitado pelo cliente
 *
 * @param next Ponteiro para a Struct BlogOperation que carregará as informações da próxima ação do blog.
 */
void makeChoice(struct BlogOperation *next)
{
    char input[100]; // Ajuste o tamanho conforme necessário
    char command[20];

    next->client_id = id;

    while (1)
    {
        /* code */
        printf("Digite a ação: ");
        fgets(input, sizeof(input), stdin);

        sscanf(input, "%s", command);

        if (strcmp(command, "subscribe") == 0)
        {
            char topicName[50];
            sscanf(input, "%s %s", command, topicName);

            next->operation_type = 4;
            next->server_response = 0;
            strcpy(next->topic, topicName);
            strcpy(next->content, "");
            break;
        }
        else if (strcmp(command, "publish") == 0)
        {
            char topic[50];
            char content[2048];
            sscanf(input, "%s %s %s", command, NULL, topic);
            fgets(content, sizeof(content), stdin);

            next->operation_type = 2;
            next->server_response = 0;
            strcpy(next->topic, topic);
            strcpy(next->content, content);
            break;
        }
        else if (strcmp(command, "list") == 0)
        {
            next->operation_type = 3;
            next->server_response = 0;
            strcpy(next->content, "");
            strcpy(next->topic, "");
            break;
        }
        else if (strcmp(command, "exit") == 0)
        {
            next->operation_type = 5;
            next->server_response = 0;
            strcpy(next->content, "");
            strcpy(next->topic, "");
            break;
        }
        else if (strcmp(command, "unsubscribe") == 0)
        {
            char topicName[50];
            sscanf(input, "%s %s", command, topicName);
            next->operation_type = 6;
            next->server_response = 0;
            strcpy(next->topic, topicName);
            strcpy(next->content, "");
            break;
        }
        else
        {
            printf("Comando inválido! Tente Novamente");
        }
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

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
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
                if (mov.server_response == 1 && mov.operation_type != 2)
                {
                    // açoes com resposta do servidor (caso tenha algo a ser feito)
                }
            }
        }
    }
    else if (pid == 0)
    {
        // Thread paralela que busca constantemente novos posts
        for (;;)
        {
            size_t count = recv(sock, &mov, sizeof(struct BlogOperation), 0);

            if (count < 0)
            {
                perror("Erro ao receber dados do servidor");
            }
            else if (count == 0)
            {
            }
            else
            {
                if (mov.server_response == 1 && mov.operation_type == 2)
                {
                    printf("\nnew post added in %s by %d\n%s", mov.topic, mov.client_id, mov.content);
                }
            }
        }
    }

    close(sock);

    return 0;
}