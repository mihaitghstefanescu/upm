CC=gcc
CFLAGS=-Wall

all: server

server: open62541.o server.o

open62541.o: open62541.c open62541.h

server.o: server.c open62541.c open62541.h

clean:
	rm -f server open62541.o server.o

run: server
	./server