/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "parse.h"
#include "log.h"

/* SYSTEM CALLS  HEADERS */
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 255

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

    struct parse_info_t info;
    int flags = parse_cmd(argc - 1, &argv[1], &info);
    printf("Flags: %x\n", flags);

    if (flags & FLAG_PATH) {
        printf("Path: %s\n", info.path);
    }
    if (flags & FLAG_BSIZE) {
        printf("Block Size: %d\n", info.block_size);
    }
    if (flags & FLAG_MAXDEPTH) {
        printf("Max Depth: %d\n", info.max_depth);
    }

    init_log();
    char *a = "hello\n";
    char *b = "worldds\n";
    char *c = "test\n";
    char *d = "all\n";
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    write_log(a);
    write_log(b);
    write_log(c);
    write_log(d);
    close_log();
    // free memory
    free(info.path);

    return 0;
}
