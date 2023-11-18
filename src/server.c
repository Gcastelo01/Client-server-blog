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

static const int MAXPENDING = 5;

/**
 * @brief Mostra para o usuário a maneira correta de executar o programa
 */
void use()
{
    printf("./server <PROTOCOL v4 || v6> <PORT> -i <INPUT FILE>");
}

/**
 * @brief Caso um tópico não exista, adiciona um novo tópico à Lista de Tópicos
 *
 * @param op Ordem de operação enviada pelo cliente
 * @param tList Lista de tópicos
 *
 */
void addTopic(struct BlogOperation *op, struct Topic *tList)
{
    if (tList->next_topic == NULL && tList->id == -1)
    {
        strcat(op->topic, "\n");
        strcpy(tList->name_topic, op->topic);

        tList->subscribers[op->client_id] = 1;
        tList->id = tList->id++;
    }
    else if (tList->next_topic == NULL)
    {
        struct Topic *new = malloc(sizeof(struct Topic));

        strcat(op->topic, "\n");
        strcpy(new->name_topic, op->topic);

        new->subscribers[op->client_id] = 1;
        new->id = tList->id++;
        new->next_topic = NULL;
        tList->next_topic = new;
    }
    else
    {
        addTopic(op, tList->next_topic);
    }
}

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
 * @param tList POnteiro para a lista de tópicos exitentes
 */
void subscribe(struct BlogOperation *op, struct Topic *tList)
{
    struct Topic *search = findTopic(tList, op->topic);

    if (search == NULL)
    {
        addTopic(op, tList);
    }
    else
    {
        search->subscribers[op->client_id] = 1;
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
 * @brief Gera uma lista de tópicos existentes, e salva ela no campo content da operação.
 *
 * @param op Struct de comunicação cliente/servidor
 * @param t Lista de Tópicos
 * @param tnames string contendo nomes dos tópicos
 */
void generateTopicList(struct BlogOperation *op, struct Topic *t, char *tnames)
{
    if (t->next_topic == NULL)
    {
        strcat(tnames, t->name_topic);
        strcpy(op->content, tnames);
    }
    else
    {
        strcat(tnames, t->name_topic);
        generateTopicList(op, t, tnames);
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
    char tnames[] = "";
    generateTopicList(op, t, tnames);
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

        printf("Adicionei o cliente %d", op->client_id);
        op->operation_type = 1;
        strcpy(op->topic, "");
        strcpy(op->content, "");

        break;

    // Novo post em um tópico
    case 2:
        createPost(op);
        printf("new post added in %s by %d\n%s", op->topic, op->client_id, op->content);
        break;

    // Listagem de Tópicos
    case 3:
        listTopics(op, tp);
        break;

    // Inscrição em um tópico (Cria novo caso topico n exista)
    case 4:
        subscribe(op, tp);
        break;

    // Desinscrever de um topico
    case 6:
        unsubscribe(op, tp);
        break;

    // Desconecta de servidor
    case 5:
        break;

    // Tratamento de erros
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    char *C_PROTOCOL = argv[1];
    in_port_t PORT = atoi(argv[2]);


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
    tpcs.id = NULL;

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
        }
    }

    return 0;
}