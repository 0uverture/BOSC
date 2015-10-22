CC = gcc -ggdb -g -O0
OBJS = producerconsumer.o list.o
LIBS = -pthread

fifo: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -rf *.o fifo
