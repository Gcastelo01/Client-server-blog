#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "blogoperation.h"

int id = 0;
int sock;

void verify(ssize_t count)
{
    if (count != sizeof(struct BlogOperation))
    {
        perror("Falha ao enviar dados para o servidor");
        exit(EXIT_FAILURE);
    }
}

void *watcher(void *data)
{
    pthread_detach(pthread_self());

    struct BlogOperation mov;

    while (1)
    {
        size_t count = recv(sock, &mov, sizeof(struct BlogOperation), 0);

        verify(count);

        if (mov.operation_type == 2)
        {
            printf("\nNew post added in %s by %d\n%s", mov.topic, mov.client_id, mov.content);
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

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

    mov.client_id = 0;
    mov.operation_type = 1;

    size_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);

    verify(count);

    ssize_t count2 = recv(sock, &mov, sizeof(struct BlogOperation), 0);

    verify(count2);

    id = mov.client_id;

    pthread_t pid2;

    pthread_create(&pid2, NULL, watcher, NULL);

    while (1)
    {
        char input[100]; // Ajuste o tamanho conforme necessário
        char command[20];

        while (1)
        {
            printf("Digite a ação: ");
            fgets(input, sizeof(input), stdin);

            sscanf(input, "%s", command);

            if (strcmp(command, "subscribe") == 0)
            {
                char topicName[50];
                char useless[2];
                sscanf(input, "%s %s %s", command, useless, topicName);

                mov.operation_type = 4;
                mov.server_response = 0;
                strcpy(mov.topic, topicName);
                strcpy(mov.content, "");

                ssize_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
                verify(count);

                break;
            }
            else if (strcmp(command, "publish") == 0)
            {
                char topic[50];
                char content[2048];
                char useless[2];

                sscanf(input, "%s %s %s", command, useless, topic);

                // Usar fgets para ler o conteúdo sem incluir quebras de linha
                printf("Digite o conteúdo do post: ");
                fgets(content, sizeof(content), stdin);

                mov.operation_type = 2;
                mov.server_response = 0;
                strcpy(mov.topic, topic);
                strcpy(mov.content, content);

                ssize_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
                
                verify(count);

                break;
            }
            else if (strcmp(command, "list") == 0)
            {
                mov.operation_type = 3;
                mov.server_response = 0;
                strcpy(mov.content, "");
                strcpy(mov.topic, "");

                ssize_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
                verify(count);

                ssize_t recvBits = recv(sock, &mov, sizeof(struct BlogOperation), 0);

                if (recvBits != sizeof(struct BlogOperation))
                    perror("Listagem de tópicos falhou!");

                printf("\n%s\n", mov.content);

                break;
            }
            else if (strcmp(command, "exit") == 0)
            {
                mov.operation_type = 5;
                mov.server_response = 0;
                strcpy(mov.content, "");
                strcpy(mov.topic, "");

                ssize_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
                verify(count);

                close(sock);
                return 0;
            }
            else if (strcmp(command, "unsubscribe") == 0)
            {
                char topicName[50];
                sscanf(input, "%s %s", command, topicName);
                mov.operation_type = 6;
                mov.server_response = 0;
                strcpy(mov.topic, topicName);
                strcpy(mov.content, "");
                break;

                ssize_t count = send(sock, &mov, sizeof(struct BlogOperation), 0);
                verify(count);
            }
            else
            {
                printf("Comando inválido! Tente Novamente\n");
            }
        }
    }

    close(sock);

    return 0;
}