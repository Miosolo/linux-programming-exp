#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

int FileToFile(char *source, char *dest, struct stat *pstat);
int DirToDir(char *source, char *dest);
int mycp(int argc, char* argv[]);
void * mycp_thread_wrapper(void * args);