#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>

typedef struct state {
  int *resource;
  int *available;
  int **max;
  int **allocation;
  int **need;
} State;

// Global variables
int m, n;
State *s = NULL;

// Mutex for access to state.
pthread_mutex_t state_mutex;

/* Random sleep function */
void Sleep(float wait_time_ms)
{
  // add randomness
  wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
  usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}

void print_available(){
	printf("Now available: ");
	int i;
	for (i = 0; i < n; ++i)
	{
		printf("%d ", s->available[i]);
	}
	printf("\n");
}
/*Returns a pseudo-random integer between 0 and given limit.*/
int random_number(int lim){
	return lim * (((double)rand() / (double)RAND_MAX) + 0.5 );
}

/* Allocate resources in request for process i, only if it 
   results in a safe state and return 1, else return 0 */
int resource_request(int i, int *request)
{
	print_available();
	printf("Process %d Requesting: %d %d %d\n", i, request[0], request[1], request[2]);
	int j;
	for (j = 0; j < n; ++j)
	{
		if(request[j] > s->available[j]){ printf("Request denied.\n"); return 0; }
	}

	for (j = 0; j < n; ++j)
	{
		s->available[j] -= request[j];
		s->need[i][j] -= request[j];
		s->allocation[i][j] += request[j];
	}
	printf("Request approved.\n");
	print_available();
 	
	request[0] = 0;
	request[1] = 0;
	request[2] = 0;

 	return 1;
}

/* Release the resources in request for process i */
void resource_release(int i, int *request)
{
	int j;
	for (j = 0; j < n; ++j)
	{
		s->available[j] += request[j];
		s->need[i][j] += request[j];
		s->allocation[i][j] -= request[j];
	}
	printf("Resources released by process %d\n", i);
	print_available();
}

/* Generate a request vector */
void generate_request(int i, int *request)
{
  	int j;
  	while(is_empty(request)){
    	for (j = 0;j < n; j++) {
    		printf("S need is %d\n", s->need[i][j]);
      		request[j] = random_number(s->need[i][j]);
    	}
	}
}

/* Generate a release vector */
void generate_release(int i, int *request)
{
  int j;
  	while(is_empty(request)){
    	for (j = 0;j < n; j++) {
      		request[j] = random_number(s->allocation[i][j]);
    	}
	}
	printf("Releasing resources %d %d %d\n", request[0], request[1], request[2]);
}

int nested_array_malloc(int n, int m, int** *arr)
{
	*arr = malloc(m * sizeof(int *));
	if (*arr == NULL){ printf("Allocation failed."); return 0;}
	int k;
	for(k = 0; k < m; k++){
		(*arr)[k] = malloc(n * sizeof(int));
		if ((*arr)[k] == NULL){ printf("Allocation failed."); return 0;}
	}
	return 1;
}

/*Returns true if given resource array only contains 0*/
int is_empty(int* arr){
	int i;
	for (i = 0; i < n; ++i)
	{
		if(arr[i] != 0) return 0; 
	}
	return 1;
}

/*Checks if given state is legal*/
int check_state(State ss){
	int i, j, procflag = 1;
	for(i = 0; i < m; i++){
		if(!is_empty(ss.need[i])){
			for (j = 0; j < n; ++j)
			{
				//If not enough resources are available, set flag to false.
				if((ss.available[j] - ss.need[i][j]) < 0) procflag = 0;
			}
			if(procflag){
				//Enough resources can be allocated to a process.
				printf("Simulating completion of process %d\n", i);
				for (j = 0; j < n; ++j)
				{
					//Simulate process finishing and releasing all resources.
					ss.need[i][j] = 0;
					ss.available[j] = ss.available[j] + ss.allocation[i][j];
					ss.allocation[i][j] = 0;
				}
				procflag = 1;
				//Check if new state is legal.
				return check_state(ss);
			}		
		return 0;
		}
	}
	return 1;
}

/* Threads starts here */
void *process_thread(void *param)
{
  /* Process number */
  int i = (int) (long) param, j;
  /* Allocate request vector */
  int *request = malloc(n*sizeof(int));
  if (request == NULL) {printf("Memory allocation failed for s\n"); return(0); }

  while (1) {
    /* Generate request */
    generate_request(i, request);
    while (!resource_request(i, request)) {
      /* Wait */
    	printf("Request denied. Sleeping.\n");
      Sleep(100);
    }
    /* Generate release */
    generate_release(i, request);
    /* Release resources */
    resource_release(i, request);
    /* Wait */
    printf("Big sleep for process %d.\n", i);
    Sleep(1000);
  }
  free(request);
}

int main(int argc, char* argv[])
{
//Rand test
/*	int swag, zero = 0, one = 0, two = 0, wtf;
	for (swag = 0; swag < 1000; ++swag)
	{
		wtf = random_number(1);
		printf("Wtf is %d\n", wtf);
		if(wtf){
			one++;
		}
		else {
			zero++;
		}
	}
	printf("Ones: %d Zeroes: %d\n", one, zero);
	exit(0);*/


  /* Get size of current state as input */
  int i, j;
  printf("Number of processes: ");
  scanf("%d", &m);
  printf("Number of resources: ");
  scanf("%d", &n);

  /* Allocate memory for state */
  s = malloc(sizeof(State)); 
  if (s == NULL) { printf("Memory allocation failed for s\n"); exit(0); };
  
  s->resource = (int*) malloc(n*sizeof(int));
  if (s->resource == NULL){ printf("Memory allocation failed.\n"); exit(0); };

  s->available = (int*) malloc(n*sizeof(int));
  if (s->available == NULL){ printf("Memory allocation failed.\n"); exit(0); };

  nested_array_malloc(n, m, &s->max);
  nested_array_malloc(n, m, &s->allocation);
  nested_array_malloc(n, m, &s->need);

  /* Get current state as input */
  printf("Resource vector: ");
  for(i = 0; i < n; i++)
  	scanf("%d", &s->resource[i]);
  printf("Enter max matrix: ");
  for(i = 0;i < m; i++)
    for(j = 0;j < n; j++)
      scanf("%d", &s->max[i][j]);
  printf("Enter allocation matrix: ");
  for(i = 0; i < m; i++)
    for(j = 0; j < n; j++) {
      scanf("%d", &s->allocation[i][j]);
    }
  printf("\n");

  /* Calcuate the need matrix */
  for(i = 0; i < m; i++)
    for(j = 0; j < n; j++)
      s->need[i][j] = s->max[i][j] - s->allocation[i][j];

  /* Calcuate the availability vector */
  for(j = 0; j < n; j++) {
    int sum = 0;
    for(i = 0; i < m; i++)
      sum += s->allocation[i][j];
    s->available[j] = s->resource[j] - sum;
  }

  /* Output need matrix and availability vector */
  printf("Need matrix:\n");
  for(i = 0; i < n; i++)
    printf("R%d ", i+1);
  printf("\n");
  for(i = 0; i < m; i++) {
    for(j = 0; j < n; j++)
      printf("%d  ",s->need[i][j]);
    printf("\n");
  }
  printf("Availability vector:\n");
  for(i = 0; i < n; i++)
    printf("R%d ", i+1);
  printf("\n");
  for(j = 0; j < n; j++)
    printf("%d  ",s->available[j]);
  printf("\n");

  //THE DEEP COPY ZONE
  //Create deep copy containing available, allocation and need for the check_state simulation.
  State *sdc;
  sdc = malloc(sizeof(State));
  sdc->available = (int*) malloc(n*sizeof(int));
  nested_array_malloc(n, m, &sdc->allocation);
  nested_array_malloc(n, m, &sdc->need);

  for (i = 0; i < m; ++i)
  {
  	for (j = 0; j < n; ++j)
  	{
  		sdc->need[i][j] = s->need[i][j];
  		sdc->allocation[i][j] = s->allocation[i][j];
  		sdc->available[j] = s->available[j];  		 
  	}
  }
	//THE DEEP COPY ZONE

  /* If initial state is unsafe then terminate with error */
  if(check_state(*sdc)) { printf("Initial state is legal.\n"); }
  else { 
  	printf("Initial state is illegal. Please check your input.\n");
  	exit(EXIT_FAILURE);
  }

  /* Seed the random number generator */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);

  //srand(time(NULL));

  /* Create m threads */
  pthread_t *tid = malloc(m*sizeof(pthread_t));
  for (i = 0; i < 1; i++)
    pthread_create(&tid[i], NULL, process_thread, (void *) (long) i);
  	
  /* Wait for threads to finish */
  pthread_exit(0);
  free(tid);

  /* Free state memory */
}
