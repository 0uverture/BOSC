#ifndef SUMSQRT_H
#define SUMSQRT_H

typedef struct work {
	int tid;
	int start;
	int end;
	double result;
} Work;

double sum_sqrt(int n, int tnum);

#endif