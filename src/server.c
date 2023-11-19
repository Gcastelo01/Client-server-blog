#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "blogoperation.h"
#include "serverClient.h"
#include "connData.h"
#include "topic.h"

static const int MAXPENDING = 5;

/**
 * @brief Inscreve um cliente em um determinado tópico, caso o mesmo exista. Se o tópico Não existe, cria um novo tópico e adiciona o cliente nele.
 *
 * @param op Ponteiro para a ordem de operação do cliente
 * @param tList Ponteiro para a lista de tópicos exitentes
 */
void subscribe(struct BlogOperation *op, struct Topic *tList)
{
    if (tList->id == -1)
    {
        tList->id = 0;
        strcpy(tList->name_topic, op->topic);
        (tList->subscribers[op->client_id]) = 1;
    }
    else
    {
        for (struct Topic *aux = tList; aux != NULL; aux = aux->next_topic)
        {

            if (strcmp(op->topic, aux->name_topic) == 0)
            {
                (aux->subscribers[op->client_id]) = 1;
                break;
            }
            else if (aux->next_topic == NULL)
            {
                struct Topic *new = malloc(sizeof(struct Topic));

                strcat(op->topic, "\n");
                strcpy(new->name_topic, op->topic);

                (new->subscribers[op->client_id]) = 1;

                new->id = aux->id + 1;
                new->next_topic = NULL;

                aux->next_topic = new;
                break;
            }
        }
    }
}

/**
 * @brief Cancela a Incrição de um cliente em um tópico
 *
 * @param op Ponteiro para a ordem de operação do cliente
 * @param tList POnteiro para a lista de tópicos exitentes
 */
void unsubscribe(struct BlogOperation *op, struct Topic *tList)
{
    for (struct Topic *aux = tList; aux != NULL; aux = aux->next_topic)
    {
        if (strcmp(aux->name_topic, op->topic) == 0)
        {
            aux->subscribers[op->client_id] = 0;
            break;
        }
    }
}

/**
 * @brief lista os tópicos existentes
 *
 * @param op Ordem de operação que será enviada com a resposta para o cliente (Ponteiro)
 * @param t Lista de tópicos
 */
void listTopics(struct BlogOperation *op, struct Topic *t)
{
    char topics[2048] = "\n";

    if (t->id != -1)
    {
        for (struct Topic *aux = t; aux != NULL; aux = aux->next_topic)
        {
            strcat(topics, aux->name_topic);
        }
    }
    else
    {
        strcat(topics, "não há topicos");
    }

    strcpy(op->content, topics);
}

/**
 * @brief Cria um novo post em um determinado tópico, caso o mesmo exista.
 *
 * @attention
 */
void createPost(struct BlogOperation *op)
{
}

/**
 * @brief Função recursiva para encontrar o último cliente adicionado, e adicionar um novo cliente ao servidor
 *
 * @param cli Ponteiro para o primeiro cliente da lista de clientes (Os clientes formam uma lista encadedada)
 *
 * @return Id do cliente que foi adicionado.
 */
int findLastAdded(struct Client *cli, int csock)
{
    if (cli->nextClient == NULL)
    {
        struct Client *aux = malloc(sizeof(struct Client));

        aux->id = cli->id++;
        aux->nextClient = NULL;
        cli->nextClient = aux;
        cli->csock = csock;

        return aux->id;
    }

    return findLastAdded(cli->nextClient, csock);
}

/**
 * @brief: Adiciona um cliente ao servidor.
 *
 * @return: Id do novo cliente adicionado.
 */
int addClient(struct Client *cli, int csock)
{
    if (cli->id == 0)
    {
        return findLastAdded(cli, csock);
    }
    else
    {
        cli->id = 0;
        cli->csock = csock;
        cli->nextClient = NULL;
        return 0;
    }
}

/**
 * @brief De acordo com o comando recebido pela entrada, executa a açao pedida pelo cliente
 *
 * @param op Dados da operação requerida pelo cliente
 * @param cli Lista encadeada de clientes cadastrados no servidor
 */
void processaEntrada(struct BlogOperation *op, struct Client *cli, struct Topic *tp, int csock, pthread_mutex_t *mutex)
{
    int opID = op->operation_type;
    op->server_response = 1;

    switch (opID)
    {
    // Novo cliente conectou ao servidor
    case 1:
        pthread_mutex_lock(mutex);
        op->client_id = addClient(cli, csock);
        pthread_mutex_unlock(mutex);

        printf("Client %d connected. \n", op->client_id);
        op->operation_type = 1;
        strcpy(op->topic, "");
        strcpy(op->content, "");

        break;

    // Novo post em um tópico
    case 2:
        pthread_mutex_lock(mutex);
        createPost(op);
        pthread_mutex_unlock(mutex);

        printf("New post added in %s by %d\n %s \n", op->topic, op->client_id, op->content);
        break;

    // Listagem de Tópicos
    case 3:
        listTopics(op, tp);

        printf("Topics Listed by %d \n", op->client_id);
        break;

    // Inscrição em um tópico (Cria novo caso topico n exista)
    case 4:
        pthread_mutex_lock(mutex);
        subscribe(op, tp);
        pthread_mutex_unlock(mutex);

        printf("Client %d subscribed to topic %s", op->client_id, op->topic);
        break;

    // Desinscrever de um topico
    case 6:
        pthread_mutex_lock(mutex);
        unsubscribe(op, tp);
        pthread_mutex_unlock(mutex);

        printf("Client %d desubscribed from topic %s", op->client_id, op->topic);
        break;

    // Desconecta de servidor
    case 5:
        printf("Client %d disconnected", op->client_id);
        break;

    // Tratamento de erros
    default:
        break;
    }
}

void *clientThread(void *data)
{
    struct connData *cdata = (struct connData *)data;
    struct BlogOperation mov;

    for (;;)
    {
        size_t numbytesrec = recv(cdata->csock, &mov, sizeof(struct BlogOperation), 0);

        if (numbytesrec < 0)
        {
            perror("Erro na recepção");
        }
        else if (numbytesrec == 0)
        {
            printf("Cliente encerrou a conexão\n");
        }
        else
        {
            processaEntrada(&mov, cdata->cli, cdata->tpcs, cdata->csock, cdata->mutex);
            send(cdata->csock, &mov, sizeof(struct BlogOperation), 0);
        }
    }

    close(cdata->csock);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{

    int PROTOCOLO;
    int servSock;

    if (argc < 3 || argc > 4)
    {
        printf("Parâmetros: <Protocolo (v4 ou v6)> <Porta do Servidor>\n");
        return 1;
    }

    char *C_PROTOCOL = argv[1];
    in_port_t PORT = atoi(argv[2]);

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

    struct sockaddr_in servAddr;            // Local address
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure

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

    struct Client *cli = malloc(sizeof(struct Client));
    struct Topic *tpcs = malloc(sizeof(struct Topic));

    tpcs->id = -1;
    tpcs->next_topic = NULL;

    for (int i = 0; i < 10; i++)
    {
        tpcs->subscribers[i] = 0;
    }

    pthread_mutex_t mutex;

    while (1)
    {
        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);
        int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);

        if (clntSock == -1)
        {
            perror("Erro na conexão accept()");
            exit(EXIT_FAILURE);
        }

        struct connData *cdata = malloc(sizeof(struct connData));

        if (!cdata)
        {
            perror("Não foi possível criar cdata");
            exit(EXIT_FAILURE);
        }

        cdata->csock = clntSock;
        cdata->cli = cli;
        cdata->tpcs = tpcs;
        cdata->mutex = &mutex;

        pthread_t tid;
        pthread_create(&tid, NULL, clientThread, cdata);
    }

    return 0;
}