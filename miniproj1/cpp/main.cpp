#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Not marked as volatile on purpose
static int x, y, i, j;

static uint64_t run_count = 1;
static uint64_t i_zero_count = 0;
static uint64_t j_zero_count = 0;
static uint64_t both_zero_count = 0;

void sigint_handler(int sig) {
  printf("%lu runs, i=0 count: %lu, j=0 count: %lu, i=j=0 count %lu\n",
         run_count - 1, i_zero_count, j_zero_count, both_zero_count);
  _exit(sig);
}

void *t1(void *) {
  x = 1;
  i = y;
  return nullptr;
}

void *t2(void *) {
  y = 1;
  j = x;
  return nullptr;
}

int main() {
  signal(SIGINT, sigint_handler);
  while (1) {
    y = 0;
    x = 0;
    printf("RUN %lu: ", run_count++);
    pthread_t pth1, pth2;

    pthread_create(&pth1, NULL, t1, NULL);
    pthread_create(&pth2, NULL, t2, NULL);

    pthread_join(pth1, NULL);
    pthread_join(pth2, NULL);
    printf("i=%d, j=%d\n", i, j);
    fflush(stdout);
    if (i == 0)
      i_zero_count++;
    if (j == 0)
      j_zero_count++;
    if (i == 0 && j == 0)
      both_zero_count++;
  }
}
