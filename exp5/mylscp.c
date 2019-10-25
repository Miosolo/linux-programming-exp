#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "mycp.h"

int mycp_caller_wrapper(char *source, char *dest) {
  sleep(1);  // wait 1s to show status better
  int stdout_dup = dup(STDOUT_FILENO);
  close(STDOUT_FILENO);  // close stdout to redirect to the black hole
  int nullfd = open("/dev/null", O_WRONLY);  // assign to stdout

  pthread_t trd;
  char *argv[3] = {"", source, dest};
  if (pthread_create(&trd, NULL, mycp_thread_wrapper, (void *)argv) != 0) {
    perror("creating cp thread");
  }

  int *rtnp;
  if (pthread_join(trd, (void **)&rtnp) != 0) {
    perror("retriving thread return value");
  }
  // recover stdout
  close(nullfd);
  dup2(stdout_dup, STDOUT_FILENO);
  close(stdout_dup);
  return *rtnp;
}

char *get_perm(mode_t m) {
  int i;
  unsigned int mask = 0x7;
  char *result = (char *)malloc(10);

  static char *perm[] = {"---", "--x", "-w-", "-wx",
                         "r--", "r-x", "rw-", "rwx"};
  for (i = 3; i > 0; i--) {
    sprintf(&result[9 - 3 * i], "%3s", perm[(m >> (i - 1) * 3) & mask]);
  }
  return result;
}

char get_type(mode_t m) {
  if (S_ISDIR(m)) {
    return ('d');
  } else if (S_ISLNK(m)) {
    return ('l');
  } else {
    return ('-');
  }
}

int main(int argc, char *argv[]) {
  // input param checking
  // 1: source
  // 2: destination
  if (argc != 3) {
    printf(
        "Input param: 1: source; 2: destination.\n Please check your input.\n");
    return 1;
  } else {
    char *source = argv[1], *dest = argv[2];
    // ls
    DIR *cwd = opendir(source);
    if (cwd == NULL) {
      perror("error: cannot open directory: ");
      return 1;
    }

    // iter through all files
    char timebuf[256];
    char dirbuf[256];
    struct dirent *entry;
    while ((entry = readdir(cwd)) != NULL) {
      if (entry->d_name[0] == '.')
        continue;  // omit . and ..

      sprintf(dirbuf, "%s/%s", source, entry->d_name);
      struct stat curstat;
      if ((lstat(dirbuf, &curstat)) != 0) {
        // fail
        perror("error: cannot get file attributes: ");
        continue;
      }

      strftime(timebuf, 255, "%Y-%m-%d %H:%M:%S",
               localtime(&(curstat.st_ctime)));
      printf("%c%s %2d %9s %9s %7d %s %s ", get_type(curstat.st_mode),
             get_perm(curstat.st_mode), curstat.st_nlink,
             getpwuid(curstat.st_uid)->pw_name,
             getgrgid(curstat.st_gid)->gr_name, curstat.st_size, timebuf,
             entry->d_name);
      fflush(stdout);  // write out the imcomplete line

      // calls mycp
      char destbuf[256];
      // mycp limit: destination should be a dir
      sprintf(destbuf, "%s/%s", dest,
              S_ISDIR(curstat.st_mode) ? entry->d_name : "");
      if (mycp_caller_wrapper(dirbuf, destbuf) == 0) {
        printf("\033[32m[success]\033[0m\n");
      } else {
        printf("\033[31m[fail]\033[0m\n");
      }
    }  // continue loop
  }
}
