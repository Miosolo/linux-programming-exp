#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shm_common.h"
#include "semaphore.h"

#define SEM_KEY 0xabc
#define QUEUE_SZ 5
#define TEXT_SZ 64

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *arry;
};

void group_kill(int sig) {
  printf("\nbye\n");
  kill(0, SIGKILL);  // kill all the process group
}

int main(int argc, char *argv[]) {
  int producer_cnt = argc >= 2 ? atoi(argv[1]) : 2;
  if (producer_cnt >= 50) {
    printf("producer should be no more than 50");
    return 1;
  }
  int consumer_cnt = argc >= 3 ? atoi(argv[2]) : 2;
  if (consumer_cnt >= 50) {
    printf("consumer should be no more than 50");
    return 1;
  }

  printf("[configeration]: Q=%d, Producer=%d, Consumer=%d\n", QUEUE_SZ,
         producer_cnt, consumer_cnt);

  // pipes & fd set
  fd_set pipes_in;
  FD_ZERO(&pipes_in);
  int producer_pipes[producer_cnt][2], consumer_pipes[consumer_cnt][2];
  int max_fd = 0;

  for (int i = 0; i < producer_cnt; i++) {
    if (pipe(producer_pipes[i]) == -1) {
      perror("opening pipe");
    }
    FD_SET(producer_pipes[i][0], &pipes_in);
    max_fd = producer_pipes[i][0] > max_fd ? producer_pipes[i][0] : max_fd;
  }
  for (int i = 0; i < consumer_cnt; i++) {
    if (pipe(consumer_pipes[i]) == -1) {
      perror("opening pipe");
    }
    FD_SET(consumer_pipes[i][0], &pipes_in);
    max_fd = consumer_pipes[i][0] > max_fd ? consumer_pipes[i][0] : max_fd;
  }

  // init semaphores
  // sem[0]: full, sem[1]: empty
  int sem = semget((key_t)SEM_KEY, 2, 0666 | IPC_CREAT);
  if (sem == -1) {
    perror("initiating semaphore");
  }

  // set sem init value
  union semun semctl_union;
  semctl_union.val = QUEUE_SZ;
  semctl(sem, 0, SETVAL, semctl_union);  // full
  semctl_union.val = 0;
  semctl(sem, 1, SETVAL, semctl_union);  // empty

  // spawning
  for (int i = 0; i < producer_cnt; i++) {
    int pid = fork();
    if (pid == 0) {
      // child
      dup2(producer_pipes[i][1], STDOUT_FILENO);  // replace pipe of stdout
      execv("./producer", NULL);
    }
    if (pid == -1) {
      perror("spawning producer");
    }
  }

  for (int i = 0; i < consumer_cnt; ++i) {
    int pid = fork();
    if (pid == 0) {
      // child
      dup2(consumer_pipes[i][1], STDOUT_FILENO);  // replace pipe of stdout
      execv("./consumer", NULL);
    } else if (pid == -1) {
      perror("spawning consumer");
    }
  }

  signal(SIGINT, group_kill);  // register the signal handler

  // reading from pipes connected to childs
  // message format: "[$qslot] $msg"

  // flags of activity
  char queue_flag[QUEUE_SZ];  // flags not string: prod/cons/0
  memset(queue_flag, 0, sizeof(queue_flag));
  char queue_msg[QUEUE_SZ][TEXT_SZ];  // msg from childs
  char msgbuf[TEXT_SZ + 10];          // msg exchange buffer read from pipes

  fd_set select_set = pipes_in;
  int qslot;
  bool first_round = true;  // controls refresh

  for (; select(max_fd + 1, &select_set, NULL, NULL, NULL) != -1;
       select_set = pipes_in) {
    // // reading phase
    // // natural mutex restrain of r & w
    // for (int i = 0; i < producer_cnt; i++) {
    //   if (FD_ISSET(producer_pipes[i][0], &select_set)) {
    //     read(producer_pipes[i][0], msgbuf, sizeof(msgbuf));
    //     sscanf(msgbuf, "[%d] %s", &qslot, msgbuf);
    //     queue_flag[qslot] = 'p';
    //     strncpy(queue_msg[qslot], msgbuf, TEXT_SZ);
    //   }
    // }
    // for (int i = 0; i < producer_cnt; i++) {
    //   if (FD_ISSET(consumer_pipes[i][0], &select_set)) {
    //     read(consumer_pipes[i][0], msgbuf, sizeof(msgbuf));
    //     sscanf(msgbuf, "[%d] %s", &qslot, msgbuf);
    //     queue_flag[qslot] = 'c';
    //     strncpy(queue_msg[qslot], msgbuf, TEXT_SZ);
    //   }
    // }

    // // output phase
    // // return to top
    // if (!first_round) {
    //   for (int l = 0; l < QUEUE_SZ; l++) {
    //     printf("\033[A");
    //   }
    //   fflush(stdout);
    // }

    // // print out
    // for (int l = 0; l < QUEUE_SZ; l++) {
    //   if (queue_flag[l] == 'p') {
    //     printf("[%d] <-[producer] %-64s\n", l, queue_msg[l]);
    //   } else if (queue_flag[l] == 'c') {
    //     printf("[%d] [consumer]-> %-64s\n", l, queue_msg[l]);
    //   } else {
    //     //printf("[%d]%-78s\n", " "); // empty;
    //   }
    // }

    // // cleanup
    // memset(queue_flag, 0, sizeof(queue_flag));

  }
  perror("selecting");
}