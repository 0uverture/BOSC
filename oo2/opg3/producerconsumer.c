#include <stdio>
#include <semaphore.h>
#include <sys/time.h>
#include <list.h>

#define PRODUCTIONS_PER_PRODUCER 5;
#define CONSUMPTIONS_PER_CONSUMER 5;

// Semaphores
sem_t mutex, empty, full;

// Item ID with mutex
pthread_mutex_t mutexItemId;
int ITEM_ID;

int in = 0, out = 0; // Legacy code ???

// FIFO list
List *fifo = list_new();
int BUFFER_SIZE; // FIFO list max size @ production/consumption

sem_init(&mutex, 0, 1);
sem_init(&empty, 0, BUFFER_SIZE);
sem_init(&full, 0, 0);

void *produce(void *data)
{
	int producerId = data;
	item nextProduced;
	for (i = 0; i < PRODUCTIONS_PER_PRODUCER; i++) {
		/* Produce the ith item */
		sem_wait(&empty);
		sem_wait(&mutex);
			// Generate item name
			pthread_mutex_lock(&mutexItemId);
			char *itemName;
			sprintf(itemName, "Item_%d", ITEM_ID++);
			pthread_mutex_unlock(&mutexItemId);
			// Produce the item
			list_add(fifo, node_new_str(itemName));
			// Print
			printf("Producer %d produced %s. Items in buffer: %d (Out of %d)\n",
				producerId, itemName, list->len, BUFFER_SIZE);
		sem_post(&mutex);
		sem_post(&full);
	}
}

void consume(void *data)
{
	int consumerId = data;
	item nextConsumed;
	for (i = 0; i < CONSUMPTIONS_PER_CONSUMER; i++) {
		sem_wait(&full);
		sem_wait(&mutex);
			// Declare item name holder
			char *itemName;
			// Produce the item
			itemName = list_remove(fifo);
			// Print
			printf("Consumer %d consumed %s. Items in buffer: %d (Out of %d)\n",
				consumerId, itemName, list->len, BUFFER_SIZE);
		sem_post(&mutex);
		sem_post(&empty);
	 	/* Consume the item */
	}
}

int main(int argc, char *argv[])
{
	// Validate arguments
	if (argc < 4 || 
		!atoi (argv[1]) ||
		!atoi (argv[2]) ||
		!atoi (argv[3]) ||
		argv[1] < 1 ||
		argv[2] < 1 ||
		argv[3] < 1) {
		printf("Invalid arguments\n");
		exit(EXIT_FAILURE);
	}

	// Retrieve arguments
	char *producerAmount = atoi(argv[1]);
	char *consumerAmount = atoi(argv[2]);
	BUFFER_SIZE = atoi(argv[3]);

	printf("Program start: %d producers, %d consumers and a buffer size of %d...\n",
		producerAmount, consumerAmount, BUFFER_SIZE);

	// Spawn threads
	
}

/* Random sleep function */
void Sleep(float wait_time_ms)
{
	wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
	usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}