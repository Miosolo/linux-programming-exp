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
      -1) {  //创建共享内存
    fprintf(
        stderr,
        "shmget failed\n");  //第二个参数，size以字节为单位指定需要共享的内存容量
    exit(EXIT_FAILURE);
  }
  if ((sem_id = semget(SEM_SEED, 1, 0666 | IPC_CREAT)) == -1) {  //信号的创建
    fprintf(stderr, "semget failed\n");
    exit(EXIT_FAILURE);
  }
  if ((shared_memory = shmat(shm_id, 0, 0)) ==
      (void
           *)-1) {  //启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间。
    fprintf(
        stderr,
        "shmat failed\n");  //第二个参数通常为空，表示让系统来选择共享内存的地址。
    exit(EXIT_FAILURE);  //第三个参数，shm_flg是一组标志位，通常为0。
  }
  shared_st = (shared_use_st *)shared_memory;
  shared_st->end_flag = 0;
  set_semvalue(sem_id, 1);
  while (shared_st->end_flag == 0) {
    semaphore_p(sem_id);
    printf("intput:");
    scanf("%s", shared_st->shm_sp);
    shared_st->pid = getpid();
    semaphore_v(sem_id);
    if (strcmp(shared_st->shm_sp, "quit") == 0) {
      shared_st->end_flag = 1;
    }
  }
  sleep(rand() % 2);
  del_semvalue(sem_id);
  if (shmdt(shared_memory) == -1) {  //将共享内存从当前进程中分离
    fprintf(stderr, "shmdt failed\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
