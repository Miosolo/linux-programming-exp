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

#define SEM_KEY 0xabc
#define QUEUE_SZ 5
#define TEXT_SZ 256
#define OUTPUT_LEN 40

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *arry;
};

void group_kill(int sig) {
  printf("bye\n");
  kill(0, SIGINT);  // interrupt all the process group
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

  printf("[configeration]: Q=%d, Producer=%d, Consumer=%d\n", QUEUE_SZ, producer_cnt, consumer_cnt);

  // pipes & fd set
  fd_set pipes_in;
  int producer_pipes[producer_cnt][2];
  for (int i = 0; i < producer_cnt; i++) {
    producer_pipes[i][0] = 100 + 2 * i;
    FD_SET(producer_pipes[i][0], &pipes_in);
    producer_pipes[i][1] = 101 + 2 * i;
  }
  int consumer_pipes[consumer_cnt][2];
  for (int i = 0; i < consumer_cnt; i++) {
    consumer_pipes[i][0] = 300 + 2 * i;
    FD_SET(consumer_pipes[i][0], &pipes_in);
    consumer_pipes[i][1] = 301 + 2 * i;
  }
  int max_fd = 299 + 2 * consumer_cnt;

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
      continue;
    } else if (pid == -1) {
      perror("spawning producer");
    } else {
      // child
      close(STDOUT_FILENO);
      dup(producer_pipes[i][1]);  // replace pipe of stdout
      execv("./producer", NULL);
    }
  }

  for (int i = 0; i < consumer_cnt; ++i) {
    int pid = fork();
    if (pid == 0) {
      continue;
    } else if (pid == -1) {
      perror("spawning consumer");
    } else {
      // child
      close(STDOUT_FILENO);
      dup(consumer_pipes[i][1]);  // replace pipe of stdout
      execv("./consumer", NULL);
    }
  }

  signal(SIGINT, group_kill);  // register the signal handler

  char *output_lines[QUEUE_SZ];
  for (int i = 0; i < QUEUE_SZ; i++) {
    output_lines[i] = (char *)malloc(OUTPUT_LEN);
    sprintf(output_lines[i], "[%d]|", i);
  }

  // flags of activity
  char queue_actor[QUEUE_SZ];  // flags not string
  char queue_msg[QUEUE_SZ][OUTPUT_LEN];   // msg from childs
  memset(queue_msg, 0, sizeof(queue_msg));
  fd_set select_set = pipes_in;
  char msgbuf[TEXT_SZ]; // read from pipe
  int actor;
  bool first_round = true; // controls refresh
  for (; select(max_fd + 1, &select_set, NULL, NULL, NULL) != -1;
       select_set = pipes_in) {
    // mutex restrain of r & w
    for (int i = 0; i < producer_cnt; i++) {
      if (FD_ISSET(producer_pipes[i][0], &select_set)) {
        read(producer_pipes[i][0], msgbuf, TEXT_SZ);
        sscanf(msgbuf, "[%d]%s", &actor, msgbuf);
        strncpy(queue_msg[actor], msgbuf, TEXT_SZ-1);
      }
    }
    for (int i = 0; i < producer_cnt; i++) {
      if (FD_ISSET(consumer_pipes[i][0], &select_set)) {
        read(consumer_pipes[i][0], msgbuf, TEXT_SZ);
        sscanf(msgbuf, "[%d]%s", &actor, msgbuf);
        strncpy(queue_msg[actor], msgbuf, TEXT_SZ-1);
      }
    }

    // return to top
    if (!first_round) {
      for (int l = 0; l < QUEUE_SZ; l++) {
        printf("\033[A");
      }
      fflush(stdout);
    }

    // refresh the output
    for (int l = 0; l < QUEUE_SZ; l++) {
      if (queue_actor[l] == 'p') {
        printf("[%d]|<-[producer] %40s\n", l, queue_msg[l]);
      } else if (queue_actor[l] == 'c') {
        printf("[%d]|[consumer]-> %40s\n", l, queue_msg[l]);
      }
    }
  }
}
