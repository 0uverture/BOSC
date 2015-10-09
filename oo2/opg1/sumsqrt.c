#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

typedef struct work {
	int tid;
	int start;
	int end;
	double result;
} Work;

void *worker(void *data) {
	Work *work = (Work *) data;

	printf("Thread %d started [%d..%d]\n",
		work->tid,
		work->start,
		work->end);

	double sum = 0;
	int i = work->start;
	while (i <= work->end) {
		sum += sqrt(i++);
	}

	printf("Thread %d finished with result %f\n",
		work->tid,
		sum);

	work->result = sum;
	pthread_exit(NULL);
}

double sum_sqrt(int n, int tnum) {
	int i;
	double sum = 0;
	
	// Array of thread ids
	pthread_t tids[tnum];

	// Array of work structs
	Work works[tnum];

	i = -1;
	while (++i < tnum) {
		// Setup work data struct for thread
		works[i].tid = i;
		works[i].start = n/tnum*i+1;
		works[i].end = i + 1 == tnum ? n : n/tnum*(i+1);

		pthread_create(&tids[i], NULL, worker, (void *) &works[i]);
	}

	i = -1;
	while (++i < tnum) {
		pthread_join(tids[i], NULL);
		sum += works[i].result;
	}

	return sum;
}

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