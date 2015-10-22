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
pthread_mutex_t pcmutex;
sem_t empty;
sem_t full;

// Item ID with mutex
pthread_mutex_t mutexItemId;
int ITEM_ID = 0;

// FIFO list
List *fifo;
int BUFFER_SIZE; // FIFO list max size @ production/consumption

/* Random sleep function */
void sleep(float wait_time_ms)
{
	// seed the random number generator
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
	usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}

void *produce(void *data)
{
	int *producerId = (int *) data;
	int i;
	for (i = 0; i < PRODUCTIONS_PER_PRODUCER; i++) {
		/* Produce the ith item */
		sem_wait(&empty);
		pthread_mutex_lock(&pcmutex);
			// Generate item name
			// 5 chars in "Item_"
			char itemName[5 + 1 + ITEM_ID/10];
			pthread_mutex_lock(&mutexItemId);
			sprintf(itemName, "Item_%d", ITEM_ID++);
			pthread_mutex_unlock(&mutexItemId);
			// Produce the item
			list_add(fifo, node_new_str(itemName));
			// Print
			printf("Producer %d produced %s. Items in buffer: %d (Out of %d)\n",
				*producerId, itemName, fifo->len, BUFFER_SIZE);
		pthread_mutex_unlock(&pcmutex);
		sem_post(&full);
		sleep(1000); // Sleep for 1 second on average
	}
	return;
}

void *consume(void *data)
{
	int *consumerId = (int *) data;
	int i;
	for (i = 0; i < CONSUMPTIONS_PER_CONSUMER; i++) {
		sem_wait(&full);
		pthread_mutex_lock(&pcmutex);
			// Consume the item
			char *itemName;
			itemName = (char *) list_remove(fifo)->elm; // Get element, expect string (void pointer)
			// Print
			printf("Consumer %d consumed %s. Items in buffer: %d (Out of %d)\n",
				*consumerId, itemName, fifo->len, BUFFER_SIZE);
		pthread_mutex_unlock(&pcmutex);
		sem_post(&empty);
		sleep((float)1000); // Sleep for 1 second on average
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

	// Retrieve arguments
	int producerAmount = atoi(argv[1]);
	int consumerAmount = atoi(argv[2]);
	BUFFER_SIZE = atoi(argv[3]);

	// Initialize list, semaphores and mutexes
	fifo = list_new();
	//list_add(fifo, node_new_str("lala"));
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&pcmutex, NULL);
	pthread_mutex_init(&mutexItemId, NULL);

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
			int *id = malloc(sizeof(int));
			*id = i;
			pthread_create(&producer_ids[i], NULL, produce, (void *) id);
			i++;
		}
		if (j < consumerAmount) {
			// Spawn consumer thread
			int *id2 = malloc(sizeof(int));
			*id2 = j;
			pthread_create(&consumer_ids[j], NULL, consume, (void *) id2);
			j++;
		}
	}
	printf("Finished spawning threads.\n");

	// Join threads
	i = 0; j = 0;
	while (i < producerAmount || j < consumerAmount) // A producer or a consumer can be spawned
	{
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
