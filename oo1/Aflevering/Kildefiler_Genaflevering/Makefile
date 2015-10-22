all: bosh

OBJS = parser.o bosh.o redirect.o
LIBS= -lreadline -ltermcap
CC = gcc -ggdb

bosh: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -rf *o bosh
