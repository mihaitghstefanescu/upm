CC=gcc
CFLAGS=-Wall

all: server

server: server.o -lopen62541

server.o: server.c

clean:
	rm -f server server.o

run: server
	./server
