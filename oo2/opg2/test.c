#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include "minunit.h"
#include "list.h"

/**
 * Total amount of threads used in each test function.
 */
#define THREAD_NUM 100

/**
 * Amount of operations perfomed by each thread in each test function.
 */
#define ACT_COUNT 10000

int tests_run = 0;


/*
 * STRUCTS
 */

/**
 * Struct describing task for worker_remove function.
 */
typedef struct remove_work {
  List *list;
  int *freq;
} Remove_Work;


/*
 * UTILITY FUNCTIONS
 */

/**
 * Removes n nodes from given list and saves the frequency
 * of node values in given freq array.
 * If list_remove return NULL, the function continues without
 * incrementing any frequency value.
 */
static void remove_n(List *list, int *freq, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    Node *node = list_remove(list);
    if (node) {
      int *value = node->elm;
      freq[*value]++;
      free(value);
    }
  }
}


/**
 * Accumulates the frequency arrays of n Remove_Work structs
 * in given work_arr array, storing the final frequency result in freq_result.
 */
static void freq_remove_work(Remove_Work *work_arr, int *freq_result, int n) {
  int i, j;

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < ACT_COUNT; ++j)
    {
      freq_result[j] += work_arr[i].freq[j];
    }
  }
}


/*
 * ASSERTION FUNCTIONS
 */

/**
 * Assertion function for executing given routine on one or more threads.
 * All threads are supplied the given data argument.
 * Amount of threads is determined by THREAD_NUM.
 * Assertion fails if pthread_create or pthread_join fails at any point.
 */
static bool assert_work(void *data, void *(*start_routine) (void *)) {
  pthread_t tid[THREAD_NUM];
  int i;

  // Create threads
  for (i = 0; i < THREAD_NUM; ++i) {
    if(0 != pthread_create(&tid[i], NULL, start_routine, data))
      return false;
  }

  // Join threads
  for (i = 0; i < THREAD_NUM; ++i) {
    if(0 != pthread_join(tid[i], NULL))
      return false;
  }

  return true;
}

/**
 * Assertion function for executing given routine on one or more threads.
 * Threads are supplied with individual elements from the given data_arr array.
 * Amount of threads is determined by THREAD_NUM.
 * Assertion fails if pthread_create or pthread_join fails at any point.
 */
static bool assert_work_arr(void *data_arr, int ele_size, void *(*start_routine) (void *)) {
  pthread_t tid[THREAD_NUM];
  int i;

  // Create threads
  for (i = 0; i < THREAD_NUM; ++i) {
    void *ele = data_arr + i * ele_size;
    if(0 != pthread_create(&tid[i], NULL, start_routine, ele))
      return false;
  }

  // Join threads
  for (i = 0; i < THREAD_NUM; ++i) {
    if(0 != pthread_join(tid[i], NULL))
      return false;
  }

  return true;
}

/**
 * Asserts that the list is empty.
 * An empty list must have a length of 0 and list_remove
 * must return NULL.
 */
static bool assert_empty(List *list) {
  return list->len == 0 && list_remove(list) == NULL;
}

/**
 * Asserts that all node values are accounted for,
 * and that no nodes were duplicated.
 * For this to be true, all values in frequency array
 * should be equal to THREAD_NUM, as each thread uses
 * any value exactly once.
 */
static bool assert_freq(int *freq, int target) {
  int i;
  for (i = 0; i < ACT_COUNT; ++i) {
    if (freq[i] != target)
      return false;
  }
  return true;
}


/*
 * THREAD WORKER FUNCTIONS
 */

/**
 * Worker function for adding ACT_COUNT amount of nodes
 * to a list, given by the data argument, on a seperate thread.
 * The nodes will hold int pointers to integers from 0 to ACT_COUNT-1.
 */
static void *worker_add(void *data) {
  List *list = data;
  int i;

  for (i = 0; i < ACT_COUNT; ++i) {
    int *value = malloc(sizeof(int));
    *value = i;

    Node *node = node_new();
    node->elm = value;

    list_add(list, node);
  }

  pthread_exit(NULL);
}

/**
 * Worker function for removing ACT_COUNT amount of nodes
 * from list.
 * The frequency of integer values contained in the nodes
 * are stored in the given frequency array. 
 */
static void *worker_remove(void *data) {
  Remove_Work *work = data;
  remove_n(work->list, work->freq, ACT_COUNT);
  pthread_exit(NULL);
}


/**
 * Test of function list_add.
 *
 * 1. Start THREAD_NUM threads each adding ACT_COUNT nodes to the list.
 * 2. Asserts that list.len is THREAD_NUM * ACT_COUNT.
 * 3. Removes and stores all node values in a frequency array.
 * 4. Asserts that list is empty.
 * 6. Asserts that each value in the frequency array is equal to THREAD_NUM.
 */
static char *test_add() {
  List *list = list_new();
  int freq[ACT_COUNT] = {};

  // Add nodes in parallel
  mu_assert(
    "Unable to create/join thread",
    assert_work(list, worker_add));

  mu_assert(
      "Invalid list length",
      THREAD_NUM * ACT_COUNT == list->len);

  // Remove all nodes and store their value frequency
  remove_n(list, freq, list->len);

  // List should now be empty
  mu_assert(
    "List not empty",
    assert_empty(list));

  mu_assert(
    "Missing/duplicated node detected",
    assert_freq(freq, THREAD_NUM));

  free(list);
  return 0;
}

/**
 * Test of function list_remove.
 *
 * 1. Adds THREAD_NUM * ACT_COUNT nodes to the list sequentially.
 * 2. Starts THREAD_NUM threads, each attempting to remove ACT_COUNT nodes
      and store the values in a frequency array local to the thread.
 * 3. Asserts that list is empty.
 * 5. Collects the frequency arrays off all threads and asserts that each value
 	  of the final frequency array is equal to THREAD_NUM.
 */
static char *test_remove() {
  List *list = list_new();
  int i, j;

  // Add nodes to list
  for (i = 0; i < THREAD_NUM; i++)
  {
    for (j = 0; j < ACT_COUNT; j++) {
      int *value = malloc(sizeof(int));
      *value = j;

      Node *node = node_new();
      node->elm = value;

      list_add(list, node);
    }
  }

  // Create work structs for threads
  Remove_Work work_arr[THREAD_NUM];
  for (i = 0; i < THREAD_NUM; ++i)
  {
    int *freq = malloc(sizeof(int) * ACT_COUNT);
    for (j = 0; j < ACT_COUNT; ++j)
    {
      freq[j] = 0;
    }

    work_arr[i].list = list;
    work_arr[i].freq = freq;
  }

  // Remove nodes in parallel
  mu_assert(
    "Unable to create/join thread",
    assert_work_arr(work_arr, sizeof(Remove_Work), worker_remove));

  // List should now be empty
  mu_assert(
    "List not empty",
    assert_empty(list));

  // Collect all frequencies into a shared array
  int freq_shared[ACT_COUNT] = {};
  freq_remove_work(work_arr, freq_shared, THREAD_NUM);

  mu_assert(
    "Missing/duplicated node detected",
    assert_freq(freq_shared, THREAD_NUM));

  // Free memory
  for (i = 0; i < THREAD_NUM; ++i)
  {
    free(work_arr[i].freq);
  }
  free(list);
	return 0;
}

/**
 * Test of function list_add and list_remove in parallel.
 * 
 * 1. Starts THREAD_NUM threads, half of which adds and half of which removes
 * 	  ACT_COUNT nodes. The removing threads store the node values in
      a frequency array local to the thread.
 * 2. Collects the frequency arrays of all threads in a shared frequency array.
 * 3. Removes and stores all remaining node values in the shared frequency array.
 * 4. Asserts that list is empty.
 * 6. Asserts that each value in the shared frequency array is equal to THREAD_NUM/2.
 */
static char *test_add_remove() {
  List *list = list_new();
  int i, j;

  // Equal amount of threads for adding and removing
  int half_thread_num = THREAD_NUM / 2;
  int thread_num = half_thread_num * 2;

  // Thread ids for add and remove workers
  pthread_t add_tids[half_thread_num];
  pthread_t rem_tids[half_thread_num];

  // Create work structs for remove workers
  Remove_Work rem_work_arr[half_thread_num];
  for (i = 0; i < half_thread_num; ++i)
  {
    int *freq = malloc(sizeof(int) * ACT_COUNT);
    for (j = 0; j < ACT_COUNT; ++j)
    {
      freq[j] = 0;
    }

    rem_work_arr[i].list = list;
    rem_work_arr[i].freq = freq;
  }

  // Start interleaved add and remove worker threads
  for (i = 0; i < thread_num; ++i)
  {
    int result;

    if (i % 2 == 0) {
      // Create add worker
      result = pthread_create(&add_tids[i/2], NULL, worker_add, list);
    } else {
      // Create remove worker
      result = pthread_create(&rem_tids[i/2], NULL, worker_remove, &rem_work_arr[i/2]);
    }

    mu_assert(
        "Unable to create thread",
        0 == result);
  }

  // Join add threads
  for (i = 0; i < half_thread_num; ++i)
  {
    mu_assert(
        "Unable to join thread",
        0 == pthread_join(add_tids[i], NULL));
  }

  // Join remove threads
  for (i = 0; i < half_thread_num; ++i)
  {
    mu_assert(
        "Unable to join thread",
        0 == pthread_join(rem_tids[i], NULL));
  }

  // Collect all frequencies from remove structs into a shared array
  int freq_shared[ACT_COUNT] = {};
  freq_remove_work(rem_work_arr, freq_shared, half_thread_num);

  // Remove and store the value of all nodes that remain in the list.
  // (Remove workers likely did remove get them all)
  remove_n(list, freq_shared, list->len);

  // List should now be empty
  mu_assert(
    "List not empty",
    assert_empty(list));

  mu_assert(
    "Missing/duplicated node detected",
    assert_freq(freq_shared, half_thread_num));

  // Free memory
  for (i = 0; i < half_thread_num; ++i)
  {
    free(rem_work_arr[i].freq);
  }
  free(list);

	return 0;
}

static char *all_tests() {
  mu_run_test(test_add);
  mu_run_test(test_remove);
  mu_run_test(test_add_remove);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
}