#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct decay_int {
  int *x;
  unsigned int executions;
  pthread_mutex_t mu;
} decay_int;

void *lifetime_observe(void *arg) {
  decay_int *ptr = (decay_int *)arg;
  printf("thread: observing %p with lifetime %d with value %d\n", ptr,
         ptr->executions, *(ptr->x));

  while (ptr->executions > 0) {
  }
  printf("thread: freeing %p with lifetime %d\n", ptr, ptr->executions);
  free(ptr->x);
  printf("setting %p decay int %p to NULL\n", (void *)ptr, ptr->x);
  ptr->x = NULL;
  pthread_exit(NULL);
  return 0;
}

void lifetime(decay_int *ptr, int executions) {
  // these opeartions execute on a separate thread
  pthread_t thread_id;
  ptr->executions = executions;
  pthread_create(&thread_id, NULL, lifetime_observe, (void *)ptr);
}

void decay_int_init(decay_int *ptr, int x) {
  pthread_mutex_lock(&ptr->mu);
  *(ptr->x) = x;
  pthread_mutex_unlock(&ptr->mu);
}

void decay_int_decay(decay_int *ptr) {
  pthread_mutex_lock(&ptr->mu);
  printf("LOG: executions: %d | value %d\n", ptr->executions - 1, *(ptr->x));
  ptr->executions--;
  pthread_mutex_unlock(&ptr->mu);
}

typedef enum opr { incr, decr } opr;

void decay_int_operation(decay_int *ptr, opr operation) {
  pthread_mutex_lock(&ptr->mu);
  switch (operation) {
  case incr: {
    *(ptr->x) += 1;
    break;
  }
  case decr: {
    *(ptr->x) -= 1;
    break;
  }
  default:
    break;
  }
  decay_int_decay(ptr);
  pthread_mutex_unlock(&ptr->mu);
  usleep(10);
}

void decay_int_get(decay_int *ptr) {
  pthread_mutex_lock(&ptr->mu);
  printf("Main get: %d\n", *(ptr->x));
  pthread_mutex_unlock(&ptr->mu);
}

int main(void) {
  // decay_int *ptr = (decay_int *)malloc(sizeof(decay_int));
  decay_int ptr;
  ptr.x = (int *)malloc(sizeof(int));
  decay_int_init(&ptr, 5);
  lifetime(&ptr, 3);
  printf("Main: decay int %p with lifetime %d and value %d\n", (void *)&ptr,
         ptr.executions, *(ptr.x));

  while (1) {
    decay_int_get(&ptr);
    decay_int_operation(&ptr, incr);
    usleep(1000000);
  }

  return 0;
}
