#ifndef __SHM_COMMON_H__
#define __SHM_COMMON_H__

#define SHM_SEED 1234
#define MAX_SHM_SIZE 2048

typedef struct shared_use_st {
  int end_flag;               // flag to indicate end of IPC
  int read_flag;              // flag to indicate the msg is consumed or not
  char shm_sp[MAX_SHM_SIZE];  // a string buffer
  int src_pid;                // pid of source / producer
} shared_use_st;

#endif
