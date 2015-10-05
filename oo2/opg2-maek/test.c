#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include "minunit.h"
#include "list.h"

// Total amount of threads used in each test function.
#define THREAD_NUM 100

// Amount of operations perfomed by each thread in each test function.
#define ACT_COUNT 1000


int tests_run = 0;

void *worker_add(void *data) {
  List *list = data;

  int i = -1;
  while (++i < ACT_COUNT) {
    Node *node = node_new();
    node->elm = (void *) (intptr_t) i;

    list_add(list, node);
  }

  pthread_exit(NULL);
}

static char * test_add() {
  int i;
  List *list = list_new();
  pthread_t tid[THREAD_NUM];

  // Create worker threads
  i = -1;
  while (++i < THREAD_NUM) {
    mu_assert(
      "Thread creation failed",
      0 == pthread_create(&tid[i], NULL, worker_add, (void *) list));
  }

  // Join worker threads
  i = -1;
  while (++i < THREAD_NUM) {
    mu_assert(
      "Thread join failed",
      0 == pthread_join(tid[i], NULL));
  }

  // Assert that all thread pushes are accounted for in list.len member.
  mu_assert(
    "Invalid list length (list.len)",
    THREAD_NUM * ACT_COUNT == list->len);

  // Initialize array for storing all pushed nodes.
  int result[ACT_COUNT];
  i = -1;
  while (++i < ACT_COUNT) {
    result[i] = 0;
  }

  // Pop all nodes, and register their value.
  i = -1;
  while (++i < THREAD_NUM * ACT_COUNT) {
    int value = (intptr_t) list_remove(list)->elm;
    result[value]++;
  }

  // Assert that all nodes have been popped.
  mu_assert(
    "Invalid list length (list_remove count)",
    list_remove(list) == NULL);

  // Assert that no nodes are missing or duplicated.
  int acc = 0;
  i = -1;
  while (++i < ACT_COUNT) {
    acc += result[i];
    mu_assert(
      "Missing/duplicated node detected",
      result[i] == THREAD_NUM);
  }

  return 0;
}


void *worker_remove(void *data) {

}

static char * test_remove() {
  return 0;
}


void *worker_add_remove(void *data) {

}

static char * test_add_remove() {
  return 0;
}


static char * all_tests() {
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