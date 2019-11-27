#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;

#ifdef __GNUC__
/* 这里使用的是GCC编译器对CAS指令封装之后的一个内建函数 */
#define atomic_cas(p, old, new) __sync_bool_compare_and_swap(p, old, new)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#error "atomic operations are supported only by GCC"
#endif

#define SHM_KEY 123456

struct attr_node {
  unsigned int id;
  unsigned int value;
};

void *shm_create(uint64_t shm_key, int ele_size, int ele_count) {
  uint32_t size = ele_count * ele_count;

  /* 创建共享内存 */
  int shmid = shmget(shm_key, size, IPC_CREAT);
  if (shmid < 0) {
    perror("shmget error :");
    return NULL;
  }

  /* 映射地址 */
  void *addr = shmat(shmid, NULL, 0);
  if (addr == (void *)-1) {
    perror("shmat error :");

    /* 删除shmid */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
      perror("shmctl IPC_RMID error :");
    }

    return NULL;
  }

  return addr;
}

int shm_write(void *shm_addr, int id, uint32_t value, uint32_t old_value) {
  struct attr_node *node = (struct attr_node *)shm_addr + id;

  int success = atomic_cas(&(node->value), old_value, value);
  if (success) {
    return 0;
  } else {
    return -1;
  }
}

int shm_read(void *shm_addr, void *buf, int id) {
  void *start_addr = (struct attr_node *)shm_addr + id;
  memcpy(buf, start_addr, sizeof(struct attr_node));

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: ./write id\n");
    return -1;
  }

  void *shm_addr = shm_create(SHM_KEY, sizeof(struct attr_node), 10);
  if (shm_addr == NULL) {
    printf("sq_create error\n");
    return -1;
  }

  struct attr_node *node = NULL;
  node = malloc(sizeof(struct attr_node));
  if (node == NULL) {
    printf("malloc error\n");
    return -1;
  }

  int id = 0;
  if (strcmp(argv[1], "clear") == 0) {
    id = atoi(argv[2]);
    shm_read(shm_addr, node, id);
    shm_write(shm_addr, id, 0, node->value);

    shm_read(shm_addr, node, id);
    printf("id = %d\nnode->value = %u\n", id, node->value);
    return 0;
  } else {
    id = atoi(argv[1]);
  }

  uint32_t count = 0;
  uint32_t i = 0;
  for (i = 0; i < 100000; i++) {
    /* 如果更新失败，重新读取最新的值再次重试，也可以限制一个重试次数 */
    for (;;) {
      shm_read(shm_addr, node, id);
      uint32_t old_value = node->value;
      ++node->value;
      int ret = shm_write(shm_addr, id, node->value, old_value);
      if (ret == 0) break;

      count++;
    }
  }

  printf("conflict count = %d\n", count);

  shm_read(shm_addr, node, id);
  printf("node->value = %u\n", node->value);

  return 0;
}