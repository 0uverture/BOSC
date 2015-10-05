#include <stdlib.h>
#include <pthread.h>
#include "minunit.h"
#include "list.h"

int tests_run = 0;

static char * test_foo() {
	mu_assert("error", 7==5);
	return 0;
}

int main(int argc, char **argv) {
	char *result = tests_foo();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}