#ifndef __SHM_COMMON_H__
#define __SHM_COMMON_H__

#define SHM_SEED 1000
#define MAX_SHM_SIZE 2048

typedef struct shared_use_st {
  int end_flag;  //用来标记进程间的内存共享是否结束: 0, 未结束； 1， 结束
  char shm_sp[MAX_SHM_SIZE];  //共享内存的空间
  int pid;                    //标注当前进程的id
} shared_use_st;

#endif
