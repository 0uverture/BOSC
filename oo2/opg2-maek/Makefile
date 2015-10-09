CC = gcc -ggdb
LIBS= -pthread 

all: fifo test

fifo: main.o list.o
	${CC} -o $@ ${LIBS} list.c main.c

test: test.o list.o
	${CC} -o $@ ${LIBS} list.c test.c;

clean:
	rm -rf *o fifo test
