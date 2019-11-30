#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include "semaphore.h"
#include "shm_common.h"

int main(int argc, char *argv[]) {
  int shm_id;
  int sem_id;
  void *shared_memory;
  shared_use_st *shared_st;

  // creates a shared mem area
  if ((shm_id = shmget(SHM_SEED, sizeof(shared_use_st), 0666 | IPC_CREAT)) ==
      -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  // creates a sem
  if ((sem_id = semget(SEM_SEED, 1, 0666 | IPC_CREAT)) == -1) {
    perror("semget");
    exit(EXIT_FAILURE);
  }
  // attachs the shm
  if ((shared_memory = shmat(shm_id, 0, 0)) == (void *)-1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  shared_st = (shared_use_st *)shared_memory;
  shared_st->end_flag = 0;  // blind write
  shared_st->read_flag = 1; // forced reset
  set_semvalue(sem_id, 1);  // allows production

  while (shared_st->end_flag == 0) {
    semaphore_p(sem_id);
    if (shared_st->read_flag) {
      // content has been consumed
      printf("[%d]input: ", getpid());
      scanf("%s", shared_st->shm_sp);
      shared_st->read_flag = 0;
      shared_st->src_pid = getpid();
    }
    semaphore_v(sem_id);
    if (strcmp(shared_st->shm_sp, "quit") == 0) {
      shared_st->end_flag = 1;
    }
  }

  sleep(rand() % 2);
  del_semvalue(sem_id);
  // detachs the shm
  if (shmdt(shared_memory) == -1) {
    perror("shmdt");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
