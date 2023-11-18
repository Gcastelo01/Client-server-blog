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

void testCount(size_t c)
{
    if (c != sizeof(struct BlogOperation))
    {
        perror("Erro ao enviar dados");
        exit(EXIT_FAILURE);
    }
}

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
            char useless[2];

            sscanf(input, "%s %s %s", command, useless, topic);
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
    size_t sizeOp = sizeof(struct BlogOperation);

    if (res < 0)
    {
        perror("Connection failed");
    }
    else
    {
        for (int i = 1; i < 6; i++)
        {
            mov.client_id = 0;
            mov.operation_type = i;

            size_t count = send(sock, &mov, sizeOp, 0);

            testCount(count);

            size_t rcvCount = recv(sock, &mov, sizeOp, 0);

            testCount(rcvCount);

            printf("Operação %d realizada com sucesso. Continuar para operação seguinte? y/n", mov.operation_type);
            scanf("%s", NULL);
        }
    }

    close(sock);

    return 0;
}