#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 100

static pthread_cond_t pWait = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pLock = PTHREAD_MUTEX_INITIALIZER;

int N = 0;
int nsecs = 2;
int *pFork;  // 01 array presents the forks; 1 -> available

int isAvailable(int i) { return (pFork[i] && pFork[(i + 1) % N]) ? 1 : 0; }

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
  char buf[MAXLINE];
  vsnprintf(buf, MAXLINE, fmt, ap);
  if (errnoflag)
    snprintf(buf + strlen(buf), MAXLINE - strlen(buf), ": %s", strerror(error));
  strcat(buf, "\n");
  fflush(stdout); /* in case stdout and stderr are the same */
  fputs(buf, stderr);
  fflush(NULL); /* flushes all stdio output streams */
}

void err_quit(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
  exit(1);
}

void takeFork(int i) {
  if (pthread_mutex_lock(&pLock) != 0) {
    perror("locking mutex");
  }
  // got the mutex
  while (1) {
    if (isAvailable(i)) {
      pFork[i] = 0;
      pFork[(i + 1) % N] = 0;
      break;
    } else {
      pthread_cond_wait(&pWait,
                        &pLock);  // wait for mutex and check forks again
    }
  }
  printf("[%d] got the forks\n", i);
  if (pthread_mutex_unlock(&pLock) != 0) {
    perror("unlocking mutex");
  }  // release the mutex
}

void putFork(int i) {
  // start without the mutex but with both forks
  if (pthread_mutex_lock(&pLock) != 0) {
    perror("locking mutex");
  }
  // with mutex
  pFork[i] = 1;
  pFork[(i + 1) % N] = 1;
  if (pthread_mutex_unlock(&pLock) != 0) {
    perror("unlocking mutex");
  }
  printf("[%d] dropped his forks\n", i);
  pthread_cond_broadcast(&pWait);
}

void thinking(int i, int nsecs) {
  printf("\033[32m[%d] thinking\033[0m\n", i);
  sleep(nsecs);
}

void eating(int i, int nsecs) {
  printf("\033[31m[%d] eating\033[0m\n", i);
  sleep(nsecs);
}
void *philosopher(void *arg) {
  int order = (long)arg;
  // start without any fork
  while (1) {
    thinking(order, nsecs);
    takeFork(order);
    eating(order, nsecs);
    putFork(order);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  int i, err;
  pthread_t tid[20];
  pthread_attr_t attr;
  if (argc < 2) err_quit("useage: philosopher_th N <-t nsecs>");
  if (argc == 4 && (strcmp(argv[2], "-t")) == 0) {
    nsecs = atoi(argv[3]);
  }
  N = atoi(argv[1]);
  if (N < 2) err_quit("N is the number of philosopher\n");
  if (N > 20) err_quit("too many philosopher");
  pFork = (int *)malloc(N * sizeof(int));
  for (i = 0; i < N; i++) {
    pFork[i] = 1;
  }
  err = pthread_attr_init(&attr);  // detach
  if (err != 0) err_quit("pthread arr init error");
  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (err != 0) err_quit("set detachstate error");
  for (i = 0; i < N; i++) {
    err = pthread_create(&tid[i], &attr, philosopher, (void *)(long)i);
    if (err != 0) err_quit("can't create thread: %s\n", strerror(err));
  }
  pause();
  return 0;
}
