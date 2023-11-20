# Client/Server blog
## O que é?

O projeto Client/Server Blog consiste em um sistema que segue o padrão publish/subscribe para simular um blog que funcionaria via linha de comando.
O blog permite a criação de diversos tópicos, listagem de tópicos, inscrição e desinscrição em um determinado tópico e, finalmente, postagens.
O programa foi inteiramente desenvolvido em C padrão, utilizando a biblioteca sockets e pthreads.

## Servidor
O servidor foi desenvolvido de modo a executar cada cliente paralelamente, em uma thread exclusiva. Os clientes podem realizar as operações descritas acima. Uma observação relevante: Ao criar uma nova postagem, todos os clientes que forem inscritos naquele tópico recebem uma notificação com a postagem.
Em seu propósito original, só existiriam 10 clientes simultâneos no servidor, e não há memória de postagens. 

## Cliente
Um cliente envia comandos para o servidor, e, paralelametne, vigia o canal de conexão para verificar se uma nova postagem foi enviada para ele.
