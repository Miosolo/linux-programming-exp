#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
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

#define MAX_TASKS 10

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
  if (argc < 2) {
    printf(
        "Input param: 1: source; 2: destination.\n Please check your input.\n");
    return 1;
  }
  char *source = argv[1];
  // /home/{myname} by default
  char *dest = argc > 2 ? argv[2] : getenv("HOME");

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
    if (entry->d_name[0] == '.') continue;  // omit . and ..

    sprintf(dirbuf, "%s/%s", source, entry->d_name);
    struct stat curstat;
    if ((lstat(dirbuf, &curstat)) != 0) {
      // fail
      perror("error: cannot get file attributes: ");
      continue;
    }

    strftime(timebuf, 255, "%Y-%m-%d %H:%M:%S", localtime(&(curstat.st_ctime)));
    printf("%c%s %2d %9s %9s %7d %s %s ", get_type(curstat.st_mode),
           get_perm(curstat.st_mode), curstat.st_nlink,
           getpwuid(curstat.st_uid)->pw_name, getgrgid(curstat.st_gid)->gr_name,
           curstat.st_size, timebuf, entry->d_name);
    fflush(stdout);  // write out the imcomplete line

    // copy to dest in a new process
    pid_t pid = fork();
    if (pid < 0) {
      perror("failed forking copy process");
      exit(1);
    } else if (pid == 0) {
      // child
      sleep(1);        // wait 1s to show status better
      fclose(stdout);  // close stdout to redirect to the black hole
      int nullfd = open("/dev/null", O_WRONLY);
      char destbuf[256];
      // mycp limit: destination should be a dir
      sprintf(destbuf, "%s/%s", dest,
              S_ISDIR(curstat.st_mode) ? entry->d_name : "");
      if (execl("./mycp", "mycp", dirbuf, destbuf, NULL) == -1) {
        perror("failed executing copy program");
        exit(1);
      }
    } else {
      // parent
      int child_stat;
      if (wait(&child_stat) == -1) {
        perror("failed waiting the child process to terminate");
      } else if (child_stat == 0) {
        printf("\033[32m[success]\033[0m\n");
      } else {
        printf("\033[31m[fail]\033[0m\n");
      }
    }  // continue loop
  }
}