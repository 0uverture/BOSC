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

static void act_list_remove(List *list, int *result, int count) {
  int i;

  // Pop all nodes, and register their value.
  i = -1;
  while (++i < count) {
    int value = (intptr_t) list_remove(list)->elm;
    result[value]++;
  }
}

/**
 * Act function for removing all elements from a list.
 * Amount of elements removed is determined by list.len.
 *
 * WARNING:
 * This function does not ensure that the list is empty.
 * See assert_list_empty function for this purpose.
 */
static void act_list_remove_all(List *list, int *result) {
  act_list_remove(list, result, list->len);
}

static void act_list_add_all(List *list) {
  int i = -1, j;
  while (++i < THREAD_NUM) {
    j = -1;
    while (++j < ACT_COUNT) {
      Node *node = node_new();
      node->elm = (void *) (intptr_t) j;
      list_add(list, node);
    }
  }
}

/**
 * Assertion function for executing given routine on one or more threads.
 * Amount of threads is determined by THREAD_NUM.
 * Assertion fails if pthread_create or pthread_join fails at any point.
 */
static char *assert_work(List *list, void *(*start_routine) (void *)) {
  pthread_t tid[THREAD_NUM];
  int i;

  // Create worker threads
  i = -1;
  while (++i < THREAD_NUM) {
    mu_assert(
      "Thread creation failed",
      0 == pthread_create(&tid[i], NULL, start_routine, (void *) list));
  }

  // Join worker threads
  i = -1;
  while (++i < THREAD_NUM) {
    mu_assert(
      "Thread join failed",
      0 == pthread_join(tid[i], NULL));
  }

  return 0;
}

/**
 * Asserts thats list.len is equal to the maximum possible length.
 * This criteria is based on THREAD_NUM and ACT_COUNT.
 */
static char *assert_list_length_max(List *list) {
  mu_assert(
      "Invalid list length",
      THREAD_NUM * ACT_COUNT == list->len);
  return 0;
}

static char *assert_list_length_min(List *list) {
  mu_assert(
    "List.len is not 0",
    0 == list->len);
  return 0;
}

/**
 * Asserts that the list is empty based on list_remove.
 */
static char *assert_list_empty(List *list) {
  mu_assert(
    "List not empty",
    list_remove(list) == NULL);
   return 0;
}

/**
 * Asserts that all node values are accounted for,
 * and that no nodes were duplicated.
 * The length of the result list should be equal to the
 * maximum node.elm value in list.
 */
static char *assert_list_integrity(int *result) {
  int i = -1;
  while (++i < ACT_COUNT) {
    mu_assert(
      "Missing/duplicated node detected",
      result[i] == THREAD_NUM);
  }
}

static void *worker_add(void *data) {
  List *list = data;

  int i = -1;
  while (++i < ACT_COUNT) {
    Node *node = node_new();
    node->elm = (void *) (intptr_t) i;

    list_add(list, node);
  }

  pthread_exit(NULL);
}

static char *test_add() {
  List *list = list_new();

  // List for storing node data.
  int result[ACT_COUNT];

  // Zero out list
  int i = -1;
  while (++i < ACT_COUNT) {
    result[i] = 0;
  }
  
  // Do adding work on threads and assert successful thread creation and joining.
  assert_work(list, worker_add);

  // Assert that all thread pushes are accounted for in list.len member.
  assert_list_length_max(list);

  // Initialize array for storing data from all pushed nodes.
  act_list_remove_all(list, result);

  // Assert that all nodes have been popped.
  assert_list_empty(list);

  // Assert that no nodes are missing or duplicated.
  assert_list_integrity(result);

  return 0;
}

typedef struct remove_work {
  List *list;
  int *result;
} Remove_Work;

static void *worker_remove(void *data) {
  Remove_Work *work = data;
  act_list_remove(work->list, work->result, ACT_COUNT);
  pthread_exit(NULL);
}

static char *test_remove() {
  int i;
  List *list = list_new();
  pthread_t tid[THREAD_NUM];
  Remove_Work *works = malloc(THREAD_NUM * sizeof(Remove_Work));
  int freq[ACT_COUNT];

  act_list_add_all(list);

  i = -1;
  while (++i < ACT_COUNT) {
    freq[i] = 0;
  }

  i = -1;
  while (++i < THREAD_NUM) {
    Remove_Work *work = malloc(sizeof(Remove_Work));
    int result[ACT_COUNT];

    // Zero out list
    int j = -1;
    while (++j < ACT_COUNT) {
      result[j] = 0;
    }

    work->list = list;
    work->result = result;
    works[i] = *work;

    mu_assert(
      "Thread creation failed",
      0 == pthread_create(&tid[i], NULL, worker_remove, (void *) work));
  }


  i = -1;
  while (++i < THREAD_NUM) {
    mu_assert(
      "Thread join failed",
      0 == pthread_join(tid[i], NULL));

    int j = -1;
    while (++j < ACT_COUNT) {
      freq[works[i].result[j]]++;
    }
  }

  assert_list_length_min(list);

  assert_list_empty(list);

  assert_list_integrity(freq);

  free(works);
  return 0;
}

typedef struct dyn_remove_work {
  List *list;
  int *result;
  int length;
} Dyn_Remove_Work;

static void *worker_dyn_remove(void *data) {
  Dyn_Remove_Work *work = data;

  Node *node = list_remove(work->list);

  while (node != NULL) {
    int value = (intptr_t) node->elm;

    work->result[value]++;
    work->length++;

    node = list_remove(work->list);
  }

  pthread_exit(NULL);
}

static char *test_add_remove() {
  int i;
  List *list = list_new();
  pthread_t tid[THREAD_NUM];
  Dyn_Remove_Work rem_works[THREAD_NUM];

  i = -1;
  while (++i < THREAD_NUM) {
    if (i % 2 == 0) {
      // Create add thread
      mu_assert(
        "Thread creation failed",
        0 == pthread_create(&tid[i], NULL, worker_add, (void *) list));
    }
    else {
      // Create remove thread
      int *result = malloc(ACT_COUNT * sizeof(int));

      // Zero out list
      int j = -1;
      while (++j < ACT_COUNT) {
        result[j] = 0;
      }

      rem_works[i].list = list;
      rem_works[i].result = result;
      rem_works[i].length = 0;

      // Create add thread
      mu_assert(
        "Thread creation failed",
        0 == pthread_create(&tid[i], NULL, worker_dyn_remove, &rem_works[i]));
    }
  }

  int freq[ACT_COUNT];

  i = -1;
  while (++i < THREAD_NUM) {
    // Join all threads
    mu_assert(
      "Thread join failed",
      0 == pthread_join(tid[i], NULL));

    if (i % 2 != 0) {
      // Is a remove thread
      int j = -1;
      while (++j < ACT_COUNT) {
        freq[rem_works[i].result[j]]++;
      }
    }
  }

  act_list_remove_all(list, freq);

  assert_list_length_min(list);

  assert_list_empty(list);

  assert_list_integrity(freq);

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