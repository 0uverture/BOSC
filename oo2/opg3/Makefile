all: fifo

OBJS = list.o producerconsumer.o
LIBS= -lpthread

fifo: producerconsumer.o ${OBJS}
	gcc -o $@  ${LIBS} ${OBJS}

clean:
	rm -rf *o fifo
