#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

#include "semaphore.h"

// to set an init value of a semphamore using global sem_union
int set_semvalue(int sem_id, int value) {
  union semun sem_union;
  sem_union.val = value;
  if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
    perror("setting sem value");
    return 1;
  }
  return 0;
}

// to remove a semphamore not used any longer
void del_semvalue(int sem_id) {
  union semun sem_union;

  // may fail, but okay
  if (semctl(sem_id, 0, IPC_RMID, sem_union) ==
      -1)  // IPC_RMID：用于删除一个已经无需继续使用的信号量标识符。
    // perror("deleting semphamore");
}

// sem wait op
int semaphore_p(int sem_id) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    perror("semaphore_p");
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
    perror("semaphore_v");
    return 1;
  }

  return 0;
}
