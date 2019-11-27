#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>

#include "semaphore.h"

int set_semvalue(int sem_id, int value) {
  union semun sem_union;

  sem_union.val = value;
  if (semctl(sem_id, 0, SETVAL, sem_union) ==
      -1)      //用来把信号量初始化为一个已知的值
    return 1;  //这个值通过union
               //semun中的val成员设置，其作用是在信号量第一次使用前对它进行设置。

  return 0;
}

void del_semvalue(int sem_id) {
  union semun sem_union;

  if (semctl(sem_id, 0, IPC_RMID, sem_union) ==
      -1)  // IPC_RMID：用于删除一个已经无需继续使用的信号量标识符。
    fprintf(stderr, "Failed to delete semaphore\n");
}

int semaphore_p(int sem_id) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;  //-1，即P（等待）操作
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return 1;
  }

  return 0;
}

int semaphore_v(int sem_id) {
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = 1;  // +1，即V（发送信号）操作。
  sem_b.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return 1;
  }

  return 0;
}
