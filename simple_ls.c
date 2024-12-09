#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <time.h>
#include <unistd.h>

void num_to_attrs(mode_t n, char* buf) {
  if (S_ISREG(n)) buf[0] = '-';
  if (S_ISCHR(n)) buf[0] = 'c';
  if (S_ISDIR(n)) buf[0] = 'd';
  if (S_ISLNK(n)) buf[0] = 'l';
  if (S_ISBLK(n)) buf[0] = 'b';
  if (S_ISFIFO(n)) buf[0] = 'p';
  if (S_ISSOCK(n)) buf[0] = 's';
  buf[1] = n & S_IRUSR ? 'r' : '-';
  buf[2] = n & S_IWUSR ? 'w' : '-';
  buf[3] = n & S_IXUSR ? 'x' : '-';
  buf[4] = n & S_IRGRP ? 'r' : '-';
  buf[5] = n & S_IWGRP ? 'w' : '-';
  buf[6] = n & S_IXGRP ? 'x' : '-';
  buf[7] = n & S_IROTH ? 'r' : '-';
  buf[8] = n & S_IWOTH ? 'w' : '-';
  buf[9] = n & S_IXOTH ? 'x' : '-';
  if (!(n & S_IXOTH)) {
    buf[9] = n & S_ISVTX ? 't' : '-';
  }
  return;
}

int main(int argc, char** argv) {
  printf("\033[0;31m");
  printf("\033[0m");
  char pathname[256] = "./";
  if (argc == 3 && strcmp(argv[1], "-al") == 0) {
    pathname[0] = '\0';
    pathname[1] = '\0';
    int ind = 0;
    while (argv[2][ind] != '\0') {
      pathname[ind] = argv[2][ind];
      ind++;
    }
    if (pathname[ind - 1] != '/') {
      pathname[ind] = '/';
    }
  }
  DIR* directory = opendir(pathname);
  struct dirent* r = readdir(directory);
  while (r != NULL) {
    struct stat buf;
    char* file_name = r->d_name;
    char full_path[256] = "";
    strcat(full_path, pathname);
    strcat(full_path, file_name);
    char attrs[12] = "";
    int s = lstat(full_path, &buf);
    char xattr[1024] = "";
    ssize_t len = listxattr(full_path, xattr, 1024);
    if (len > 0) {
      attrs[10] = '+';
    }
    if (s != 0) {
      perror("Can't get stat from file");
    }
    num_to_attrs(buf.st_mode, attrs);

    struct passwd* user = getpwuid(buf.st_uid);
    struct group* user_group = getgrgid(buf.st_gid);
    char time_buf[20] = "";
    strftime(time_buf, 20, "%b %d %H:%M", localtime(&buf.st_mtime));
    if (S_ISLNK(buf.st_mode)) {
      char file_loc[256] = "";
      if (readlink(full_path, file_loc, 255) == -1) {
        perror("Can't resolve symlink");
      }
      printf("%s %ld %s %s %ld  %s %s -> %s\n", attrs, buf.st_nlink,
             user->pw_name, user_group->gr_name, buf.st_size, time_buf,
             r->d_name, file_loc);
    }
    printf("%s %ld %s %s %ld  %s %s\n", attrs, buf.st_nlink, user->pw_name,
           user_group->gr_name, buf.st_size, time_buf, r->d_name);
    r = readdir(directory);
  }
  return 0;
}
