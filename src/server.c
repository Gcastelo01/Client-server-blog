#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "blogoperation.h"
#include "serverClient.h"
#include "topic.h"

static const int MAXPENDING = 5;

/**
 * @brief Encontra um tópico na lista. Caso o tópico não exista, retorna NULL
 *
 * @param t: Ponteiro para o primeiro tópico da lista de tópicos.
 * @param name: Nome do tópico que está sendo procurado
 *
 * @return Ponteiro para o tópico procurado, NULL caso não exista.
 */
struct Topic *findTopic(struct Topic *t, char *name)
{
    if (strcmp(name, t->name_topic) == 0)
    {
        return t;
    }
    else if (t->next_topic != NULL)
    {
        return findTopic(t->next_topic, name);
    }

    return NULL;
}

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
    struct Topic *search = findTopic(tList, op->topic);

    if (search == NULL)
    {
        strcpy(op->content, "O tópico não existe");
    }
    else
    {
        search->subscribers[op->client_id] = 0;
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
void createPost(struct BlogOperation *op) {}

/**
 * @brief Função recursiva para encontrar o último cliente adicionado, e adicionar um novo cliente ao servidor
 *
 * @param cli Ponteiro para o primeiro cliente da lista de clientes (Os clientes formam uma lista encadedada)
 *
 * @return Id do cliente que foi adicionado.
 */
int findLastAdded(struct Client *cli)
{
    if (cli->nextClient == NULL)
    {
        struct Client *aux = malloc(sizeof(struct Client));

        aux->id = cli->id++;
        aux->nextClient = NULL;
        cli->nextClient = aux;

        return aux->id;
    }

    return findLastAdded(cli->nextClient);
}

/**
 * @brief: Adiciona um cliente ao servidor.
 *
 * @return: Id do novo cliente adicionado.
 */
int addClient(struct Client *cli)
{
    if (cli->id == 0)
    {
        return findLastAdded(cli);
    }
    else
    {
        cli->id = 0;

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
void processaEntrada(struct BlogOperation *op, struct Client *cli, struct Topic *tp)
{
    int opID = op->operation_type;
    op->server_response = 1;

    switch (opID)
    {
    // Novo cliente conectou ao servidor
    case 1:
        op->client_id = addClient(cli);

        printf("Client %d connected. \n", op->client_id);
        op->operation_type = 1;
        strcpy(op->topic, "");
        strcpy(op->content, "");

        break;

    // Novo post em um tópico
    case 2:
        createPost(op);
        printf("new post added in %s by %d\n %s \n", op->topic, op->client_id, op->content);
        break;

    // Listagem de Tópicos
    case 3:
        listTopics(op, tp);
        printf("Topics Listed by %d \n", op->client_id);
        break;

    // Inscrição em um tópico (Cria novo caso topico n exista)
    case 4:
        subscribe(op, tp);
        printf("Client %d subscribed to topic %s", op->client_id, op->topic);
        break;

    // Desinscrever de um topico
    case 6:
        unsubscribe(op, tp);
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

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        printf("Parâmetros: <Endereço do Servidor> <Porta do Servidor>\n");
        return 1;
    }
    char *C_PROTOCOL = argv[1];
    in_port_t PORT = atoi(argv[2]);

    int PROTOCOLO;

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

    struct sockaddr_in clntAddr;
    struct BlogOperation mov;

    struct Client cli;
    struct Topic tpcs;

    tpcs.id = -1;
    tpcs.next_topic = NULL;

    for (int i = 0; i < 10; i++)
    {
        tpcs.subscribers[i] = 0;
    }

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
            processaEntrada(&mov, &cli, &tpcs);
            send(clntSock, &mov, sizeof(struct BlogOperation), 0);
        }
    }

    return 0;
}