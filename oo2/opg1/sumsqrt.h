#ifndef SUMSQRT_H
#define SUMSQRT_H

typedef struct work {
	int tid;
	int start;
	int end;
	double result;
} Work;

void *worker(void *data);
double sum_sqrt(int n, int tnum);
int main(int argc, char* argv[]);

#endif