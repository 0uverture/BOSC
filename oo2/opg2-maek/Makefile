all: fifo test

LIBS= -lpthread

fifo: main.o ${OBJS}
	gcc -o $@ ${LIBS} list.o main.o

test: test.o
	gcc -o $@ ${LIBS} test.o;

clean:
	rm -rf *o fifo
