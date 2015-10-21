#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

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

static void print_res(int *res) {
  int i;
  printf("[DEBUG] ");
  for(i = 0; i < n; i++)
    printf("R%d ", i+1);
  printf("\n[DEBUG] ");
  for(i = 0; i < n; i++)
    printf("%d  ", s->available[i]);
  printf("\n");
  printf("\n");
}

static int **matrix_alloc() {
  int i, **matrix = malloc(m * sizeof(int *));
  for (i = 0; i < m; ++i) {
    matrix[i] = malloc(n * sizeof(int));
  }

  return matrix;
}

static void matrix_free(int **matrix) {
  int i;
  for (i = 0; i < m; ++i) {
    free(matrix[i]);
  }
  free(matrix);
}

static State *state_alloc() {
  State *state = malloc(sizeof(State));

  state->resource   = malloc(n * sizeof(int));
  state->available  = malloc(n * sizeof(int));
  state->max        = matrix_alloc();
  state->allocation = matrix_alloc();
  state->need       = matrix_alloc();

  return state;
}

static void state_free(State *state) {
  free(state->resource);
  free(state->available);
  matrix_free(state->max);
  matrix_free(state->allocation);
  matrix_free(state->need);
}

static State *state_cpy(State *state) {
  State *cpy = state_alloc();
  int i, j;

  for (i = 0; i < n; ++i) {
    cpy->resource[i]  = state->resource[i];
    cpy->available[i] = state->available[i];
  }

  for (i = 0; i < m; ++i)
    for (j = 0; j < n; ++j) {
      cpy->max[i][j]        = state->max[i][j]; 
      cpy->allocation[i][j] = state->allocation[i][j];
      cpy->need[i][j]       = state->need[i][j];
    }

  return cpy;
}

static bool vec_lte(int *v1, int *v2) {
  bool lte = true;
  int i;
  for (i = 0; i < n; ++i)
    if (v1[i] > v2[i]) {
      lte = false;
      break;
    }

  return lte;
}

static int *vec_add_ass(int *v1, int *v2) {
  int i;
  for (i = 0; i < n; ++i)
    v1[i] += v2[i];

  return v1;
}

static int *vec_sub_ass(int *v1, int *v2) {
  int i;
  for (i = 0; i < n; ++i)
    v1[i] -= v2[i];

  return v1;
}

static bool state_safe(State *state) {
  int i;

  int work[n];
  bool finish[n];
  for (i = 0; i < n; ++i) {
    work[i] = state->available[i];
    finish[i] = false;
  }

  bool progress;
  do {
    progress = false;

    for (i = 0; i < m; ++i) {
      if (finish[i]) continue;

      if (vec_lte(state->need[i], work)) {
        vec_add_ass(work, state->allocation[i]);
        finish[i] = progress = true;
      }
    }
  } while (progress);

  for (i = 0; i < n; ++i)
    if (!finish[i]) return false;

  return true;
}

static bool request_safe(int *req, int req_i, State *state) {
  if (!vec_lte(req, state->need[req_i])) {
    printf("[ERROR] P%d exceeded maximum claim", req_i);
    return false;
  }

  if (!vec_lte(req, state->available)) {
    return false;
  }

  State *cpy = state_cpy(state);
  vec_sub_ass(cpy->available, req);
  vec_add_ass(cpy->allocation[req_i], req);
  vec_sub_ass(cpy->need[req_i], req);

  bool safe = state_safe(cpy);
  state_free(cpy);
  return safe;
}

/* Random sleep function */
void Sleep(float wait_time_ms)
{
  // add randomness
  wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
  usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}

/* Allocate resources in request for process i, only if it 
   results in a safe state and return 1, else return 0 */
int resource_request(int i, int *request)
{
  pthread_mutex_lock(&state_mutex);
  printf("[INFO] Processing request from P%d\n", i);
  printf("[DEBUG] Request:\n");
  print_res(request);

  if (request_safe(request, i, s)) {
    vec_sub_ass(s->available, request);
    vec_add_ass(s->allocation[i], request);
    vec_sub_ass(s->need[i], request);

    printf("[INFO] Request from P%d accepted\n", i);
    printf("[DEBUG] Availability:\n");
    print_res(s->available);
    pthread_mutex_unlock(&state_mutex);
    return 1;
  }

  printf("[INFO] Request from P%d denied\n", i);
  printf("[DEBUG] Availability:\n");
  print_res(s->available);
  pthread_mutex_unlock(&state_mutex);
  return 0;
}

/* Release the resources in request for process i */
void resource_release(int i, int *request)
{
  pthread_mutex_lock(&state_mutex);

  vec_add_ass(s->available, request);
  vec_sub_ass(s->allocation[i], request);
  vec_add_ass(s->need[i], request);

  printf("[INFO] P%d released resources\n", i);
  printf("[DEBUG] Availability:\n");
  print_res(s->available);
  pthread_mutex_unlock(&state_mutex);
}

// http://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c
int randint(int n) {
  if (!n) return 0;

  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n;
  }
}

/* Generate a request vector */
void generate_request(int i, int *request)
{
  int j, sum = 0;
  while (!sum) {
    for (j = 0;j < n; j++) {
      request[j] = randint(s->need[i][j]);
      sum += request[j];
    }
  }
  printf("Process %d: Requesting resources.\n",i);
}

/* Generate a release vector */
void generate_release(int i, int *request)
{
  int j, sum = 0;
  while (!sum) {
    for (j = 0;j < n; j++) {
      request[j] = randint(s->allocation[i][j]);
      sum += request[j];
    }
  }
  printf("Process %d: Releasing resources.\n",i);
}

/* Threads starts here */
void *process_thread(void *param)
{
  /* Process number */
  int i = (int) (long) param, j;
  /* Allocate request vector */
  int *request = malloc(n*sizeof(int));
  while (1) {
    /* Generate request */
    generate_request(i, request);
    while (!resource_request(i, request)) {
      /* Wait */
      Sleep(100);
    }
    /* Generate release */
    generate_release(i, request);
    /* Release resources */
    resource_release(i, request);
    /* Wait */
    Sleep(1000);
  }
  free(request);
}

int main(int argc, char* argv[])
{
  /* Get size of current state as input */
  int i, j;
  printf("Number of processes: ");
  scanf("%d", &m);
  printf("Number of resources: ");
  scanf("%d", &n);

  /* Allocate memory for state */
  s = state_alloc();

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
      s->need[i][j] = s->max[i][j]-s->allocation[i][j];

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

  /* If initial state is unsafe then terminate with error */
  if (!state_safe(s)) {
    printf("[ERROR] Initial state unsafe\n");
    exit(EXIT_FAILURE);
  }

  /* Seed the random number generator */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
  
  /* Create m threads */
  pthread_t *tid = malloc(m*sizeof(pthread_t));
  for (i = 0; i < m; i++)
    pthread_create(&tid[i], NULL, process_thread, (void *) (long) i);
  
  /* Wait for threads to finish */
  pthread_exit(0);
  free(tid);

  /* Free state memory */
  state_free(s);
}
