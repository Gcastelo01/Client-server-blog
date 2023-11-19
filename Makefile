CC = gcc
CFLAGS = -Wall -lpthread

INC = ./include
SRC = ./src
TEST = ./tests
BIN = ./bin
EXEC = ./exec

${EXEC}/client: ${BIN}/client.o ${EXEC}/server
	${CC} ${CFLAGS} -I ${INC} ${BIN}/client.o -o ${EXEC}/client

${EXEC}/server: ${BIN}/server.o
	${CC} ${CFLAGS} -I ${INC} ${BIN}/server.o -o ${EXEC}/server

${BIN}/client.o: ${SRC}/client.c 
	${CC} ${CFLAGS} -I ${INC} -c ${SRC}/client.c -o ${BIN}/client.o

${BIN}/server.o: ${SRC}/server.c ${INC}/serverClient.h ${INC}/topic.h 
	${CC} ${CFLAGS} -I ${INC} -c ${SRC}/server.c -o ${BIN}/server.o


test_server:
	${CC} ${CFLAGS} -I ${INC} ${TEST}/testeServer.c -o ${EXEC}/testeser
	${CC} ${CFLAGS} -I ${INC} ${SRC}/server.c -o ${EXEC}/server


test_cli:
	${CC} ${CFLAGS} -I ${INC} ${TEST}/testeCliente.c -o ${EXEC}/testecli
	${CC} ${CFLAGS} -I ${INC} ${SRC}/client.c -o ${EXEC}/client


clean:
	rm -rf ${BIN}/*
	rm -rf ${EXEC}/*
	clear
