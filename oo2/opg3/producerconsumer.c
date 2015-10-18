#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/times.h>
#include "list.h"

int PRODUCTIONS_PER_PRODUCER = 5;
int CONSUMPTIONS_PER_CONSUMER = 5;

// Semaphores
sem_t mutex;
sem_t empty;
sem_t full;

// Item ID with mutex
pthread_mutex_t mutexItemId;
int ITEM_ID;

// FIFO list
List *fifo;
int BUFFER_SIZE; // FIFO list max size @ production/consumption

void *produce(void *data)
{
	int *producerId = (int *) data;
	int i;
	for (i = 0; i < PRODUCTIONS_PER_PRODUCER; i++) {
		printf("P: Start of forloop\n");
		/* Produce the ith item */
		sem_wait(&empty);
		printf("P: After wait4empty\n");
		sem_wait(&mutex);
		printf("P: After wait4mutex\n");
			// Generate item name
			pthread_mutex_lock(&mutexItemId); // Necessary? - Already have "mutex", which should block parallel productions
			char *itemName;
			sprintf(itemName, "Item_%d", ITEM_ID++);
			pthread_mutex_unlock(&mutexItemId);
			// Produce the item
			list_add(fifo, node_new_str(itemName));
			// Print
			printf("Producer %d produced %s. Items in buffer: %d (Out of %d)\n",
				*producerId, itemName, fifo->len, BUFFER_SIZE);
		sem_post(&mutex);
		sem_post(&full);
		Sleep(1000); // Sleep for 1 second on average
	}
	return;
}

void *consume(void *data)
{
	int *consumerId = (int *) data;
	int i;
	for (i = 0; i < CONSUMPTIONS_PER_CONSUMER; i++) {
		printf("C: Start of forloop\n");
		sem_wait(&full);
		printf("C: After wait4full\n");
		sem_wait(&mutex);
		printf("C: After wait4mutex\n");
			// Consume the item
			char *itemName;
			itemName = (char *) list_remove(fifo)->elm; // Get element, expect string (void pointer)
			// Print
			printf("Consumer %d consumed %s. Items in buffer: %d (Out of %d)\n",
				*consumerId, itemName, fifo->len, BUFFER_SIZE);
		sem_post(&mutex);
		sem_post(&empty);
		Sleep(1000); // Sleep for 1 second on average
	}
	return;
}

int main(int argc, char *argv[])
{
	// Validate arguments
	if (argc < 4 || 
		atoi (argv[1]) < 1 || // 0 if unsuccessful parse or negative or 0 if otherwise invalid :)
		atoi (argv[2]) < 1 ||
		atoi (argv[3]) < 1) {

		printf("Invalid arguments. Please provide three positive integers.\n");
		exit(EXIT_FAILURE);
	}

	// Initialize list and semaphores
	fifo = list_new();
	sem_init(&mutex, 0, 1);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);

	// Retrieve arguments
	int producerAmount = atoi(argv[1]);
	int consumerAmount = atoi(argv[2]);
	BUFFER_SIZE = atoi(argv[3]);

	printf("Program start: %d producers, %d consumers and a buffer size of %d...\n",
		producerAmount, consumerAmount, BUFFER_SIZE);

	// Arrays of thread ids
	pthread_t *producer_ids = malloc(producerAmount * sizeof(pthread_t));
	pthread_t *consumer_ids = malloc(consumerAmount * sizeof(pthread_t));

	// Attempt to spawn producers and consumers somewhat at the same time
	int i = 0, j = 0;
	while (i < producerAmount || j < consumerAmount) // A producer or a consumer can be spawned
	{
		if (i < producerAmount) {
			// Spawn producer thread
			pthread_create(&producer_ids[i], NULL, produce, (void *) &i);
			i++;
		}
		if (j < consumerAmount) {
			// Spawn consumer thread
			pthread_create(&consumer_ids[j], NULL, consume, (void *) &j);
			j++;
		}
	}
	printf("Finished spawning threads.\n");

	// Join threads
	i = 0; j = 0;
	while (i < producerAmount || j < consumerAmount) // A producer or a consumer can be spawned
	{
		printf("Trying to wait for thread\n");
		if (i < producerAmount) {
			// Join producer thread
			pthread_join(producer_ids[i], NULL);
			i++;
		}
		if (j < consumerAmount) {
			// Join consumer thread
			pthread_join(consumer_ids[j], NULL);
			j++;
		}
	}
	printf("Finished joining threads.\n");

	free(producer_ids);
	free(consumer_ids);
}

/* Random sleep function */
void Sleep(float wait_time_ms)
{
	/*
	// seed the random number generator
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
	usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
	*/
}