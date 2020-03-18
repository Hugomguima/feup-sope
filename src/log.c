#include "log.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <utils.h>
#include <errno.h>

int write_log() {
  if(getenv("LOG_FILENAME") != NULL) { // Write log to LOG_FILENAME
      printf("Write log to LOG_FILENAME");
  }
  else { // Write log to a previous file
      int fileLog = open("log.txt", O_RDONLY);
      if (fileLog == -1) {
          printf("%s", strerror(errno));
          exit(1);
      }

      if (close(fileLog) != 0) {
          printf("%s", strerror(errno));
          exit(1);
      }
  }
  return 0;
}
