#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

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

int main(int argc, char const *argv[])
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);

  int freq[10] = {};
  int i;

  while(1) {
    for (i = 0; i < 10; ++i)
    {
      freq[randint(10)]++;
    }

    for (i = 0; i < 10; ++i)
    {
      printf("%d ", freq[i]);
    }
    printf("\n");
  }

  

  return 0;
}