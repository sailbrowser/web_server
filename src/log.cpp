#include <stdio.h>
#include <stdarg.h>
#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>

void server_log(const char* format, ...) {
  va_list args;
  va_start (args, format);
  if(flock(STDOUT_FILENO, LOCK_EX) == -1) {
    perror("flock LOCK_EX");
    exit(1);
  }
  vprintf (format, args);
  if(flock(STDOUT_FILENO, LOCK_UN) == -1) {
    perror("flock LOCK_UN");
    exit(1);
  }
  va_end (args);
}
