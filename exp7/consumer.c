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
  if ((shm_id = shmget(SHM_SEED, sizeof(shared_use_st), 0666 | IPC_CREAT)) ==
      -1) {
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }
  if ((sem_id = semget(SEM_SEED, 1, 0666 | IPC_CREAT)) == -1) {
    fprintf(stderr, "semget failed\n");
    exit(EXIT_FAILURE);
  }
  if ((shared_memory = shmat(shm_id, 0, 0)) == (void *)-1) {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  shared_st = (shared_use_st *)shared_memory;
  shared_st->end_flag = 0;
  set_semvalue(sem_id, 0);
  while (shared_st->end_flag == 0) {
    semaphore_p(sem_id);
    printf("what producer input is %s\n", shared_st->shm_sp);
    printf("His pid is %d\n", shared_st->pid);
    semaphore_v(sem_id);
    if (strcmp(shared_st->shm_sp, "quit") == 0) {
      shared_st->end_flag = 1;
    }
  }
  if (shmdt(shared_memory) == -1) {
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  if (shmctl(shm_id, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
