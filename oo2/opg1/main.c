#include <stdlib.h>
#include <stdio.h>
#include "sumsqrt.h"

int main(int argc, char* argv[]) {
	int n;
	int tnum;

	if (argc < 3 || 
		!sscanf(argv[1], "%d", &n) ||
		!sscanf(argv[2], "%d", &tnum) ||
		n < tnum ||
		n < 1 ||
		tnum < 1) {
		printf("Invalid arguments\n");
		exit(EXIT_FAILURE);
	}

	printf("Summing for %d using %d thread(s)\n", n, tnum);
	printf("Result: %f\n", sum_sqrt(n, tnum));
}