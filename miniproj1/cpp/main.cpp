#include <cstddef>
#include <cstdint>
#include <exception>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

static int x = 0, y = 0, i = 2, j = 2;

static uint64_t run_count = 1;
static uint64_t both_zero_count = 0;

static pthread_barrier_t barrier;

void sigint_handler(int sig) {
  printf("\n%lu runs, i=j=0 count %lu\n", run_count - 1, both_zero_count);
  _exit(sig);
}

void *t1(void *) {
  while (true) {
    pthread_barrier_wait(&barrier);
    x = 1;
    i = y;
    pthread_barrier_wait(&barrier);
  }
  return nullptr;
}

void *t2(void *) {
  while (true) {
    pthread_barrier_wait(&barrier);
    y = 1;
    j = x;
    pthread_barrier_wait(&barrier);
  }
  return nullptr;
}

inline void handle_error(int err, const char msg[]) {
  if (err != 0) {
    fprintf(stderr, "%s failed with error code %d\n", msg, err);
    exit(1);
  }
}

int main(int argc, char **argv) {
  signal(SIGINT, sigint_handler);
  size_t N = 2;
  if (argc == 2) {
    try {
      int a = std::stoi(argv[1]);
      if (a < 2) {
        fprintf(stderr, "Invalid value N<2\n");
        exit(1);
      }
      N = a;
    } catch (std::exception const &ex) {
      fprintf(stderr, "%s\n", ex.what());
      exit(1);
    }
  }

  std::vector<pthread_t> id(N, 0);

  int err = pthread_barrier_init(&barrier, nullptr, N + 1);
  handle_error(err, "pthread_barrier_init");

  for (size_t t = 0; t < N; t++) {
    if (t % 2 == 0) {
      err = pthread_create(&id[t], nullptr, t1, nullptr);
    } else {
      err = pthread_create(&id[t], nullptr, t2, nullptr);
    }
    handle_error(err, "pthread_create");
  }

  while (1) {
    y = 0;
    x = 0;
    i = 2;
    j = 2;
    pthread_barrier_wait(&barrier);
    pthread_barrier_wait(&barrier);
    if (i == 0 && j == 0) {
      both_zero_count++;
      printf("RUN %lu: i=%d, j=%d\n", run_count, i, j);
    }

    run_count++;
  }

  for (size_t t = 0; t < N; t++) {
    err = pthread_join(id[t], nullptr);
    handle_error(err, "pthread_join");
  }

  pthread_barrier_destroy(&barrier);
  return 0;
}