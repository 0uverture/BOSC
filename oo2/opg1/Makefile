CC = gcc -ggdb
LIBS = -pthread -lm

all: sumsqrt test

sumsqrt: main.o sumsqrt.o
	${CC} -o $@ sumsqrt.c main.c ${LIBS}

test: test.o sumsqrt.o
	${CC} -o $@ sumsqrt.c test.c ${LIBS};

clean:
	rm -rf *o sumsqrt test
