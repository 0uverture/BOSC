CC = gcc -ggdb
LIBS= -lm -pthread

sumsqrt: sumsqrt.o
	${CC} -o $@ sumsqrt.o ${LIBS}

clean:
	rm -rf *.o sumsqrt
