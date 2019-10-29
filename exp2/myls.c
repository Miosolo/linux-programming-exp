#include <dirent.h>
#include <errno.h>
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
    return 'd';
  } else if (S_ISLNK(m)) {
    return 'l';
  } else if (S_ISCHR(m)) {
    return 'c';
  } else if (S_ISFIFO(m)) {
    return 'p';
  } else if (S_ISBLK(m)) {
    return 'b';
  } else if (S_ISSOCK(m)) {
    return 's';
  } else {
    return ('-');
  }
}

void ls(char *path, bool showdir) {
  DIR *cwd = opendir(path);
  if (cwd == NULL) {
    perror("error: cannot open directory: ");
    return;
  }
  if (showdir) {
    printf("%s:\n", path);
  }

  // iter through all files
  char timebuf[255];
  char dirbuf[1024];
  struct dirent *entry;
  while ((entry = readdir(cwd)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;  // omit . and ..

    sprintf(dirbuf, "%s/%s", path, entry->d_name);
    struct stat curstat;
    if ((lstat(dirbuf, &curstat)) != 0) {
      // fail
      perror("error: cannot get file attributes: ");
      continue;
    }

    strftime(timebuf, 255, "%Y-%m-%d %H:%M:%S", localtime(&(curstat.st_ctime)));
    printf("%c%s %2d %9s %9s %7d %s %s\n", get_type(curstat.st_mode),
           get_perm(curstat.st_mode), curstat.st_nlink,
           getpwuid(curstat.st_uid)->pw_name, getgrgid(curstat.st_gid)->gr_name,
           curstat.st_size, timebuf, entry->d_name);
  }
}

int main(int argc, char *argv[]) {
  // input param filling
  if (argc == 1) {
    ls(".", false);
  } else if (argc == 2) {
    ls(argv[1], false);
  } else {
    for (int i = 1; i < argc; i++) {
      ls(argv[i], true);
    }
  }
}