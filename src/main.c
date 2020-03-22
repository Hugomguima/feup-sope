/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "parse.h"
#include "utils.h"
#include "log.h"
#include "cleanup.h"

/* SYSTEM CALLS  HEADERS */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define READ_PIPE   0
#define WRITE_PIPE  1

int main(int argc, char *argv[]/*, char * envp[]*/) {
    if (argc < 2 || argc > 9) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    //int ret;

    // error | path | max-depth | S | L | B | b | a | l
    int flags;

    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_pointers(1, info.path);
        return -1;
    }

    char *path;
    int block_size;
    int max_depth;

    path = (flags & FLAG_PATH) ? info.path : ".";
    block_size = (flags & FLAG_BSIZE) ? info.block_size : 1024;
    max_depth = (flags & FLAG_MAXDEPTH) ? info.max_depth : -1;
    (void)max_depth;

    struct stat status;

    if (fget_status(path, &status)) {
        free_pointers(1, info.path);
        return -1;
    }

    file_type_t ftype = sget_type(&status);
    long fsize;
    if (flags & FLAG_BYTES) {
        fsize = status.st_size;
    } else {
        if (block_size > 512) {
            fsize = status.st_blocks / (block_size / 512);
        } else {
            fsize = status.st_blocks * (512 / block_size);
        }
    }

    switch (ftype) {
        case FTYPE_REG:
            {
                char buffer[BUFFER_SIZE];
                sprintf(buffer, "%ld""\x9""%s\n", fsize, path);
                write(STDOUT_FILENO, buffer, strlen(buffer));
            }
            break;
        case FTYPE_DIR:
            {
                DIR *dir;

                if ((dir = opendir(path)) == NULL) {
                    char error[BUFFER_SIZE];
                    char *s = strerror(errno);
                    sprintf(error, "Error %d: %s\n", errno, s);
                    write(STDERR_FILENO, error, strlen(error));
                    return errno;
                }

                struct dirent *direntp;

                while ((direntp = readdir(dir)) != NULL) {

                    // Skip . and .. directories
                    if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0) continue;

                    // Build new path
                    char new_path[BUFFER_SIZE];
                    sprintf(new_path, "%s%s%s", info.path, ((info.path[strlen(info.path) - 1] == '/') ? "" : "/"), direntp->d_name);

                    // Build command line arguments
                    parse_info_t new_info;
                    new_info.path = new_path;
                    new_info.block_size = info.block_size;
                    new_info.max_depth = (info.max_depth > 0) ? max_depth - 1 : max_depth;
                    (void)new_info;
                }


            }
        default:
            break;
    }

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
