#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#define SEM_SEED 1001

union semun  //在使用semctl函数时需要用到的
{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

int set_semvalue(int sem_id, int value);  //初始化信号
void del_semvalue(int sem_id);            //删除信号
int semaphore_p(int sem_id);              //信号p操作
int semaphore_v(int sem_id);              //信号v操作

#endif
