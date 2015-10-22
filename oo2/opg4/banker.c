#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include "minunit.h"

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

  printf("[INFO] ");
  for (i = 0; i < n; ++i) {
    printf("R%d ", i);
  }

  printf("\n[INFO] ");
  for (i = 0; i < n; ++i) {
    printf("%d  ", i);
  }

  printf("\n\n");
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
  printf("[INFO] Processing request from P%d (", i);
  int j;
  for (j = 0; j < n-1; ++j)
  {
    printf("%d, ", request[j]);
  }
  printf("%d)\n", request[n-1]);

  if (request_safe(request, i, s)) {
    vec_sub_ass(s->available, request);
    vec_add_ass(s->allocation[i], request);
    vec_sub_ass(s->need[i], request);

    printf("[INFO] Request from P%d accepted\n", i);
    printf("[INFO] Availability changed:\n");
    print_res(s->available);
    pthread_mutex_unlock(&state_mutex);
    return 1;
  }

  printf("[INFO] Request from P%d denied\n", i);
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

  printf("[INFO] P%d released resources (", i);
  int j;
  for (j = 0; j < n-1; ++j)
  {
    printf("%d, ", request[j]);
  }
  printf("%d)\n", request[n-1]);

  printf("[INFO] Availability changed:\n");
  print_res(s->available);
  pthread_mutex_unlock(&state_mutex);
}

// http://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c
int randint(int n) {
  if (!n) return 0;
  n++;

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

static bool test_matrix_eq(int mat1[n][m], int **mat2) {
  int i, j;
  for(i = 0; i < m; i++) 
    for(j = 0; j < n; j++)
      if (mat1[i][j] != mat2[i][j]) return false;

  return true;
}

static char *test() {
  m = n = 3;
  s = state_alloc();

  // Set inital state
  s->available[0] = 4;
  s->available[1] = 2;
  s->available[2] = 1;


  s->need[0][0] = 3;
  s->need[0][1] = 2;
  s->need[0][2] = 3;

  s->need[1][0] = 1;
  s->need[1][1] = 3;
  s->need[1][2] = 0;

  s->need[2][0] = 3;
  s->need[2][1] = 2;
  s->need[2][2] = 1;


  s->allocation[0][0] = 2;
  s->allocation[0][1] = 2;
  s->allocation[0][2] = 0;

  s->allocation[1][0] = 1;
  s->allocation[1][1] = 0;
  s->allocation[1][2] = 0;

  s->allocation[2][0] = 1;
  s->allocation[2][1] = 2;
  s->allocation[2][2] = 3;


  // Reqest invalid vector
  int req0[3] = { 0, 0, 1 };
  mu_assert(
    "Request was not denied",
    !resource_request(0, req0));


  // Request valid vector
  int req2[3] = { 2, 2, 1 };
  mu_assert(
    "Request was not accepted",
    resource_request(2, req2));

  // Expected resulting state
  int req2_available[3] = { 2, 0, 0 };
  int req2_need[3][3] = {
    { 3, 2, 3 },
    { 1, 3, 0 },
    { 1, 0, 0}
  };
  int req2_allocation[3][3] = {
    { 2, 2, 0 },
    { 1, 0, 0 },
    { 3, 4, 4 }
  };

  // Assert resulting state
  mu_assert(
    "Available vector invalid",
    0 == memcmp(req2_available, s->available, n));

  mu_assert(
    "Need matrix invalid",
    test_matrix_eq(req2_need, s->need));

  mu_assert(
    "Allocation matrix invalid",
    test_matrix_eq(req2_allocation, s->allocation));


  // Request valid vector
  int req1[3] = { 1, 0, 0 };
  mu_assert(
    "Request was not accepted",
    resource_request(1, req1));

  // Expected resulting state
  int req1_available[3] = { 1, 0, 0 };
  int req1_need[3][3] = {
    { 3, 2, 3 },
    { 0, 3, 0 },
    { 1, 0, 0}
  };
  int req1_allocation[3][3] = {
    { 2, 2, 0 },
    { 2, 0, 0 },
    { 3, 4, 4 }
  };

  // Assert resulting state
  mu_assert(
    "Available vector invalid",
    0 == memcmp(req1_available, s->available, n));

  mu_assert(
    "Need matrix invalid",
    test_matrix_eq(req1_need, s->need));

  mu_assert(
    "Allocation matrix invalid",
    test_matrix_eq(req1_allocation, s->allocation));


  // Release vector
  int rel2[3] = { 3, 0, 0 };
  resource_release(2, rel2);

  // Expected resulting state
  int rel2_available[3] = { 4, 0, 0 };
  int rel2_need[3][3] = {
    { 3, 2, 3 },
    { 0, 3, 0 },
    { 4, 0, 0}
  };
  int rel2_allocation[3][3] = {
    { 2, 2, 0 },
    { 2, 0, 0 },
    { 0, 4, 4 }
  };

  // Assert resulting state
  mu_assert(
    "Available vector invalid",
    0 == memcmp(rel2_available, s->available, n));

  mu_assert(
    "Need matrix invalid",
    test_matrix_eq(rel2_need, s->need));

  mu_assert(
    "Allocation matrix invalid",
    test_matrix_eq(rel2_allocation, s->allocation));


  state_free(s);
  return 0;
}

int main(int argc, char* argv[])
{
  if (argc >= 2 && strcmp(argv[1], "test") == 0) {
    char *result = test();
    if (result != 0) {
      printf("[TEST] %s\n", result);
      printf("[TEST] Test failure\n");
    }
    else {
      printf("[TEST] All tests passed\n");
    }

    return result != 0;
  }

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
