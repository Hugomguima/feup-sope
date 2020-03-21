/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "parse.h"
#include "log.h"
#include "cleanup.h"

/* SYSTEM CALLS  HEADERS */
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 255
#define READ_PIPE   0
#define WRITE_PIPE  1

int main(int argc, char *argv[], char * envp[]) {
    if (argc < 2 || argc > 9) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    int ret;

    // error | path | max-depth | S | L | B | b | a | l
    int flags;

    struct parse_info_t info;
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_pointers(1, info.path);
        return -1;
    }

    char *path;
    int block_size;

    /* Write Log
      init_log();
      char *a = "hello\n";
      sleep(3.7);
      write_log(a);
      sleep(2.90);
      write_log(a);
      close_log();
    */
    // free memory
    free_pointers(1, info.path);

    return 0;
}
