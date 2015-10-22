#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "minunit.h"
#include "sumsqrt.h"

int tests_run = 0;

static bool double_eq(double val1, double val2, double delta) {
  return fabs(val1 - val2) <= delta;
}

static char *test_thread_1_n_0() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(0, 1), 0.0, 0.0));

  return 0;
}

static char *test_thread_2_n_0() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(0, 2), 0.0, 0.0));

  return 0;
}

static char *test_thread_1_n_42() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(42, 1), 184.499, 0.001));

  return 0;
}

static char *test_thread_2_n_42() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(42, 2), 184.499, 0.001));

  return 0;
}

static char *test_thread_3_n_42() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(42, 3), 184.499, 0.001));

  return 0;
}

static char *test_thread_10_n_10() {
  mu_assert(
    "Invalid result",
    double_eq(sum_sqrt(10, 10), 22.468, 0.001));

  return 0;
}

static char *all_tests() {
  mu_run_test(test_thread_1_n_0);
  mu_run_test(test_thread_2_n_0);
  mu_run_test(test_thread_1_n_42);
  mu_run_test(test_thread_2_n_42);
  mu_run_test(test_thread_3_n_42);
  mu_run_test(test_thread_10_n_10);

  return 0;
}

int main(int argc, char const *argv[])
{
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