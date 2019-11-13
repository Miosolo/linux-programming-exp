#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TEXT_SZ 64
#define FULL_SEM_KEY 123
#define EMPTY_SEM_KEY 321
#define SHM_BASE 1000


struct shared_use_st {
  // int written;         //非0：表示可读，0表示可写
  char text[TEXT_SZ];  //记录写入和读取的文本
};

// int main() {
//   int running = 1;
//   void *shm = NULL;
//   struct shared_use_st *shared;
//   int shmid;
//   shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
//   if (shmid == -1) {
//     perror("getting shared memory identifier");
//     exit(EXIT_FAILURE);
//   }
//   shm = shmat(shmid, 0, 0);
//   if (shm == (void *)-1) {
//     perror("getting shared memory");
//     exit(EXIT_FAILURE);
//   }
//   shared = (struct shared_use_st *)shm;

//   while (running) {
//     if (shared->written != 0) {
//       printf("You wrote: %s", shared->text);
//       sleep(rand() % 3);
//       shared->written = 0;
//       if (strncmp(shared->text, "end", 3) == 0) running = 0;
//     } else
//       sleep(1);
//   }
//   if (shmdt(shm) == -1) {
//     exit(EXIT_FAILURE);
//   }
//   if (shmctl(shmid, IPC_RMID, 0) == -1) {
//     exit(EXIT_FAILURE);
//   }
//   exit(EXIT_SUCCESS);
// }

int main() {
  sleep(1);
  printf("[1] bob\n");
}