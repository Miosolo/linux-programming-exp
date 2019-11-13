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
#define SEM_KEY 0xabc

struct shared_use_st {
  // int written;         //非0：表示可读，0表示可写
  char text[TEXT_SZ];  //记录写入和读取的文本
};

// int main() {
//   int running = 1;
//   void *shm = NULL;
//   struct shared_use_st *shared = NULL;
//   char buffer[BUFSIZ + 1];
//   int shmid;
//   shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
//   if (shmid == -1) {
//     exit(EXIT_FAILURE);
//   }
//   shm = shmat(shmid, (void *)0, 0);
//   if (shm == (void *)-1) {
//     exit(EXIT_FAILURE);
//   }
//   printf("Memory attached at %X\n", (int)shm);
//   shared = (struct shared_use_st *)shm;

//   while (running) {
//     if (shared->written == 1) {
//       sleep(1);
//       printf("Waiting...\n");
//     } else {
//       printf("Enter some text: ");
//       fgets(buffer, BUFSIZ, stdin);
//       strncpy(shared->text, buffer, TEXT_SZ);
//       shared->written = 1;
//     }
//     if (strncmp(buffer, "end", 3) == 0) running = 0;
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
  printf("[2] alice\n");
}
