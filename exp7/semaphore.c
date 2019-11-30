#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>

#include "semaphore.h"

// to set an init value of a semphamore using global sem_union
int set_semvalue(int sem_id, int value) {
  union semun sem_union;
  sem_union.val = value;
  if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
    return 1;
  }
  return 0;
}

// to remove a semphamore not used any longer
void del_semvalue(int sem_id) {
  union semun sem_union;

  if (semctl(sem_id, 0, IPC_RMID, sem_union) ==
      -1)  // IPC_RMID：用于删除一个已经无需继续使用的信号量标识符。
    fprintf(stderr, "Failed to delete semaphore\n");
}

// sem wait op
int semaphore_p(int sem_id) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return 1;
  }

  return 0;
}

// sem signal op
int semaphore_v(int sem_id) {
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = 1;
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return 1;
  }

  return 0;
}
