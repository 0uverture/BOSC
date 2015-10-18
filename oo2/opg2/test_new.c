#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "minunit.h"
#include "list.h"

/**
 * Total amount of threads used in each test function.
 */
#define THREAD_NUM 100

/**
 * Amount of operations perfomed by each thread in each test function.
 */
#define ACT_COUNT 1000

int tests_run = 0;

/*
 * UTILITY FUNCTIONS
 */



/*
 * ASSERTION FUNCTIONS
 */

/**
 * Assertion function for executing given routine on one or more threads.
 * Amount of threads is determined by THREAD_NUM.
 * Assertion fails if pthread_create or pthread_join fails at any point.
 */
static char *assert_work(void *data, void *(*start_routine) (void *)) {
  pthread_t tid[THREAD_NUM];
  int i;

  // Create threads
  for (i = 0; i < THREAD_NUM; ++i) {
    mu_assert(
      "Thread creation failed",
      0 == pthread_create(&tid[i], NULL, start_routine, data));
  }

  // Join threads
  for (i = 0; i < THREAD_NUM; ++i) {
    mu_assert(
      "Thread join failed",
      0 == pthread_join(tid[i], NULL));
  }

  return 0;
}




/**
 * Test of function list_add.
 *
 * 1. Start THREAD_NUM threads each adding ACT_COUNT nodes to the list.
 * 2. Asserts that list.len is THREAD_NUM * ACT_COUNT.
 * 3. Removes and stores all node values in a frequency array.
 * 4. Asserts that list.len is 0.
 * 5. Asserts that list_remove returns NULL.
 * 6. Asserts that each value in the frequency array is equal to THREAD_NUM.
 */
static char *test_add() {


	return 0;
}

/**
 * Test of function list_remove.
 *
 * 1. Adds THREAD_NUM * ACT_COUNT nodes to the list sequentially.
 * 2. Starts THREAD_NUM threads, each attempting to remove ACT_COUNT nodes
      and store the values in a frequency array local to the thread.
 * 3. Asserts that list.len is 0.
 * 4. Asserts that list_remove returns NULL.
 * 5. Collects the frequency arrays off all threads and asserts that each value
 	  of the final frequency array is equal to THREAD_NUM.
 */
static char *test_remove() {
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
 * 4. Asserts that list.len is 0.
 * 5. Asserts that list_remove returns NULL.
 * 6. Asserts that each value in the shared frequency array is equal to THREAD_NUM.
 */
static char *test_add_remove() {
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