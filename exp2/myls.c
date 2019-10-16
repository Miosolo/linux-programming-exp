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

char* get_perm(mode_t m) {
  int i;
  unsigned int mask = 0x7;
  char *result = (char*)malloc(10);

  static char *perm[] = {"---", "--x", "-w-", "-wx",
                         "r--", "r-x", "rw-", "rwx"};
  for (i = 3; i > 0; i--) {
    sprintf(result[3-i], "%3s", perm[(m >> (i - 1) * 3) & mask]);
  }
  return result;
}

char get_type(mode_t m) {
  if (S_ISDIR(m)) {
      return('d');
    } else if (S_ISLNK(m)) {
      return('l');
    } else {
      return('-');
    }
}

bool ls(char *path, bool showdir) {
  DIR *cwd = opendir(path);
  if (cwd == NULL) {
    perror("error: cannot open directory: ");
    return false;
  }

  // iter through all files
  struct dirent *entry;
  while ((entry = readdir(cwd)) != NULL) {
    struct stat curstat;
    if ((lstat(strcat(cwd, entry->d_name), &curstat)) != 0) {
      // fail
      perror("error: cannot get file attributes: ");
      continue;
    }

    printf("%c%s %d %10s %10s %7d %s %s\n", 
           get_type(curstat.st_mode),
           get_type(curstat.st_mode),
           curstat.st_nlink,
           getpwuid(curstat.st_uid), 
           getgrgid(curstat.st_gid), 
           curstat.st_size, 
           localtime(curstat.st_ctime),
           entry->d_name);
  }
}

int main(int argc, char *argv[]) {
  // input param filling
  if (argc == 1) {
    ls(".", false);
  } else {
    for (int i = 1; i < argc; i++) {
      ls(argv[i], true);
    }
  }
}