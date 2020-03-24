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
#define LOG_FILE    2

int error_sys(char *error_msg) {
    char error[BUFFER_SIZE];
    sprintf(error, "%s\nError %d: %s\n", error_msg, errno, strerror(errno));
    write(STDERR_FILENO, error, strlen(error));
    return errno;
}

int main(int argc, char *argv[]/*, char * envp[]*/) {
    if (argc < 2) {
        errno = EINVAL;
        return error_sys("Program usage: simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]");
    }

    int subprocess = 0; // indicates if this is a subprocess or the main process
    int ppipe_write;    // pipe to write to parent in case of subprocess

    {
        struct stat stdout_status, stdin_status;

        if (fstat(STDOUT_FILENO, &stdout_status) || fstat(STDIN_FILENO, &stdin_status)) {
            return error_sys("fstat error on reading stdin and stdout status");
        }

        if (sget_type(&stdout_status) == FTYPE_FIFO && sget_type(&stdin_status) == FTYPE_FIFO) {
            int std[2 /* 3 */];
            if (read(STDIN_FILENO, std, sizeof(int) * 2 /* 3 */) != sizeof(int) * 2 /* 3 */) {
                return error_sys("read error upon reading pipe to obtain stdout and stdin");
            }
            if ((ppipe_write = dup(STDOUT_FILENO)) == -1) {
                return error_sys("dup error upon copying pipe descriptor");
            }
            if (dup2(std[READ_PIPE], STDIN_FILENO) == -1 || dup2(std[WRITE_PIPE], STDOUT_FILENO) == -1) {
                return error_sys("dup2 error upon restoring stdin and stdout");
            }

            /* set log file */

            subprocess = 1;
        } else {
            /* init log*/
        }
    }

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

    if (fget_status(path, &status, flags & FLAG_DEREF)) {
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

                fsize = 0;

                if ((dir = opendir(path)) == NULL) {
                    return error_sys("opendir error");
                }

                struct dirent *direntp;

                while ((direntp = readdir(dir)) != NULL) {

                    // Skip . and .. directories
                    if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0) continue;

                    // Build new path
                    char new_path[BUFFER_SIZE];
                    sprintf(new_path, "%s%s%s", path, ((path[strlen(path) - 1] == '/') ? "" : "/"), direntp->d_name);

                    struct stat new_status;

                    if (fget_status(new_path, &new_status, flags & FLAG_DEREF)) {
                        return error_sys("fget_status error on reading directory's file status");
                    }

                    file_type_t new_type = sget_type(&new_status);
                    long new_fsize;
                    switch (new_type) {
                        case FTYPE_REG:
                            new_fsize = fget_size(flags & FLAG_BYTES, &new_status, block_size);
                            fsize += new_fsize;
                            if ((flags & FLAG_ALL) && ((flags & FLAG_MAXDEPTH) == 0 || max_depth > 0)) {
                                char buffer[BUFFER_SIZE];
                                sprintf(buffer, "%ld""\x9""%s\n", new_fsize, new_path);
                                write(STDOUT_FILENO, buffer, strlen(buffer));
                            }
                            break;
                        case FTYPE_DIR:
                            {
                                // Build command line arguments
                                parse_info_t new_info;
                                new_info.path = new_path;
                                new_info.block_size = info.block_size;
                                new_info.max_depth = (info.max_depth > 0) ? max_depth - 1 : max_depth;

                                char **new_argv = build_argv(argv[0], flags, &new_info);

                                int pipe_ctosp[2];  // Pipe child to subprocess
                                int pipe_ctop[2];   // Pipe child to parent

                                int return_status;

                                if (pipe(pipe_ctosp) || pipe(pipe_ctop)) {
                                    return error_sys("pipe error");
                                }

                                pid_t pid = fork();

                                switch (pid) {
                                    case -1:
                                        return error_sys("fork error");
                                    case 0:
                                        {
                                            int std[2 /*3*/];
                                            if ((std[READ_PIPE] = dup(STDIN_FILENO)) == -1 || (std[WRITE_PIPE] = dup(STDOUT_FILENO)) == -1 /* || (std[LOG_FILE] = dup(log_file_fd)) == -1 */) {
                                                return error_sys("dup error upon copying stdin and stdout descriptors");
                                            }

                                            if (write(pipe_ctosp[WRITE_PIPE], std, sizeof(int) * 2 /* 3 */) == -1) {
                                                return error_sys("write error to subprocess connection pipe");
                                            }

                                            if (close(pipe_ctop[READ_PIPE]) || close(pipe_ctosp[WRITE_PIPE])) {
                                                return error_sys("close error upon closing pipe");
                                            }

                                            if (dup2(pipe_ctop[WRITE_PIPE], STDOUT_FILENO) == -1 || dup2(pipe_ctosp[READ_PIPE], STDIN_FILENO) == -1) {
                                                return error_sys("dup2 error upon redefining descriptors pointed by stdin and stdout");
                                            }

                                            if (execv(argv[0], new_argv) == -1) {
                                                return error_sys("execv error");
                                            }
                                        }
                                        break;
                                    default:
                                        {
                                            if (waitpid(pid, &return_status, 0) == -1) {
                                                return error_sys("waitpid error");
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

                                            int subdir_size = 0;
                                            if (read(pipe_ctop[READ_PIPE], &subdir_size, sizeof(int)) == -1) {
                                                return error_sys("write error upon reading from child connection pipe");
                                            }
                                            fsize += (flags & FLAG_SEPDIR) ? 0 : subdir_size;
                                        }
                                        break;
                                }
                            }
                            break;
                        case FTYPE_LINK:
                            {
                                new_fsize = fget_size(flags & FLAG_BYTES, &new_status, block_size);
                                fsize += new_fsize;
                                if ((flags & FLAG_ALL) && ((flags & FLAG_MAXDEPTH) == 0 || max_depth > 0)) {
                                    char buffer[BUFFER_SIZE];
                                    sprintf(buffer, "%ld""\x9""%s\n", new_fsize, new_path);
                                    write(STDOUT_FILENO, buffer, strlen(buffer));
                                }
                           }
                           break;
                        default:
                            break;
                    }

                }
                if ((flags & FLAG_MAXDEPTH) == 0 || max_depth >= 0) {
                    char buffer[BUFFER_SIZE];
                    sprintf(buffer, "%ld""\x9""%s\n", fsize, path);
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                }

                if (subprocess) {
                    if (write(ppipe_write, &fsize, sizeof(int)) == -1) {
                        return error_sys("write error upong writing to parent connection pipe");
                    }
                }

                if (closedir(dir)) {
                    return error_sys("closedir");
                }
            }
            break;
        case FTYPE_LINK:
            {
                // Dereference symbolic link if flag is set
                char buffer[BUFFER_SIZE];
                sprintf(buffer, "%ld""\x9""%s\n", fsize, path);
                write(STDOUT_FILENO, buffer, strlen(buffer));
           }
           break;
        default:
            break;
    }


    // free memory
    free_pointers(1, info.path);

    return 0;
}
