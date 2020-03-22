/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "cleanup.h"
#include "parse.h"
#include "utils.h"
#include "log.h"

/* SYSTEM CALLS  HEADERS */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    struct stat status;

    if (fget_status(path, &status)) {
        free_pointers(1, info.path);
        return -1;
    }

    file_type_t ftype = sget_type(&status);
    long fsize = fget_size(flags & FLAG_BYTES, &status, block_size);

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
                    sprintf(new_path, "%s%s%s", path, ((path[strlen(path) - 1] == '/') ? "" : "/"), direntp->d_name);

                    struct stat new_status;

                    if (stat(new_path, &new_status) == -1) {
                        char error[BUFFER_SIZE];
                        char *s = strerror(errno);
                        sprintf(error, "Error %d: %s\n", errno, s);
                        write(STDERR_FILENO, error, strlen(error));
                        return errno;
                    }

                    file_type_t new_type = sget_type(&new_status);
                    long new_fsize;
                    switch (new_type) {
                        case FTYPE_REG:
                            if (flags & FLAG_ALL) {
                                new_fsize = fget_size(flags & FLAG_BYTES, &new_status, block_size);
                                char buffer[BUFFER_SIZE];
                                sprintf(buffer, "%ld""\x9""%s\n", new_fsize, new_path);
                                write(STDOUT_FILENO, buffer, strlen(buffer));
                            }
                            break;
                        case FTYPE_DIR:
                            if (max_depth > 0 || max_depth == -1) {
                                // Build command line arguments
                                parse_info_t new_info;
                                new_info.path = new_path;
                                new_info.block_size = info.block_size;
                                new_info.max_depth = (info.max_depth > 0) ? max_depth - 1 : max_depth;

                                char **new_argv = build_argv(argv[0], flags, &new_info);

                                pid_t pid = fork();

                                int return_status;

                                switch (pid) {
                                    case -1:
                                        {
                                            char error[BUFFER_SIZE];
                                            char *s = strerror(errno);
                                            sprintf(error, "Error %d: %s\n", errno, s);
                                            write(STDERR_FILENO, error, strlen(error));
                                            return errno;
                                        }
                                    case 0:
                                        if (execv(argv[0], new_argv) == -1) {
                                            char error[BUFFER_SIZE];
                                            char *s = strerror(errno);
                                            sprintf(error, "Error %d: %s\n", errno, s);
                                            write(STDERR_FILENO, error, strlen(error));
                                            return errno;
                                        }
                                        return 0; // exit child
                                    default:
                                        {
                                            if (wait(&return_status) == -1) {
                                                char error[BUFFER_SIZE];
                                                char *s = strerror(errno);
                                                sprintf(error, "Error %d: %s\n", errno, s);
                                                write(STDERR_FILENO, error, strlen(error));
                                                return errno;
                                            }
                                            int i = 0;
                                            while (new_argv[i] != NULL) {
                                                free(new_argv[i]);
                                                i++;
                                            }
                                            free(new_argv);

                                            if (!WIFEXITED(return_status) || WEXITSTATUS(return_status) != 0) {
                                                write(STDERR_FILENO, "error on child\n", 16);
                                                return -1;
                                            }
                                        }
                                        break;
                                }
                            } else {
                                fsize = fget_size(flags & FLAG_BYTES, &new_status, block_size);
                                char buffer[BUFFER_SIZE];
                                sprintf(buffer, "%ld""\x9""%s\n", new_fsize, new_path);
                                write(STDOUT_FILENO, buffer, strlen(buffer));
                            }
                            break;
                        default:
                            break;
                    }

                }
                char buffer[BUFFER_SIZE];
                sprintf(buffer, "%ld""\x9""%s\n", fsize, path);
                write(STDOUT_FILENO, buffer, strlen(buffer));
            }
        default:
            break;
    }

    //Write Log
    init_log();
    char *a = "hello\n";
    sleep(3.7);
    write_log(a);
    sleep(2.90);
    write_log(a);
    close_log();

    // free memory
    free_pointers(1, info.path);

    return 0;
}
