/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "log.h"
#include "parse.h"
#include "sig_handler.h"
#include "utils.h"

/* SYSTEM CALLS  HEADERS */
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256
#define READ_PIPE 0
#define WRITE_PIPE 1
#define LOG_FILE 2

int exit_status = 0;

int error_sys(char *error_msg) {
    char error[BUFFER_SIZE];
    sprintf(error, "simpledu: %s", error_msg);
    perror(error);
    return errno;
}

void write_log_exit_status(void) {
    if (write_log_long("EXIT", exit_status)) {
        write(STDERR_FILENO, "error upon writing log\n", 23);
    }
}

int main(int argc, char *argv[] /*, char * envp[]*/) {
    if (argc < 2) {
        errno = EINVAL;
        return error_sys(
            "Program usage: simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] "
            "[--max-depth=N]");
    }
    int subprocess =
        0;            // indicates if this is a subprocess or the main process
    int ppipe_write;  // pipe to write to parent in case of subprocess
    int log_file_fd;
    struct timeval init_time;

    if (atexit(write_log_exit_status)) {
        return error_sys("error on atexit");
    }

    gettimeofday(&init_time, 0);  // Init time

    {
        struct stat stdout_status, stdin_status;

        if (fstat(STDOUT_FILENO, &stdout_status) ||
            fstat(STDIN_FILENO, &stdin_status)) {
            exit_status =
                error_sys("fstat error on reading stdin and stdout status");
            return exit_status;
        }

        if (sget_type(&stdout_status) == FTYPE_FIFO &&
            sget_type(&stdin_status) == FTYPE_FIFO) {
            int std[3];

            if (read(STDIN_FILENO, std, sizeof(int) * 3) != sizeof(int) * 3) {
                exit_status = error_sys(
                    "read error upon reading pipe to obtain stdout and stdin");
                return exit_status;
            }
            if (read(STDIN_FILENO, &init_time, sizeof(init_time)) !=
                sizeof(init_time)) {
                exit_status =
                    error_sys("read error upon reading pipe to obtain timeval");
                return exit_status;
            }
            if ((ppipe_write = dup(STDOUT_FILENO)) == -1) {
                exit_status =
                    error_sys("dup error upon copying pipe descriptor");
                return exit_status;
            }
            if (dup2(std[READ_PIPE], STDIN_FILENO) == -1 ||
                dup2(std[WRITE_PIPE], STDOUT_FILENO) == -1) {
                exit_status =
                    error_sys("dup2 error upon restoring stdin and stdout");
                return exit_status;
            }

            /* set log file */
            log_file_fd = std[LOG_FILE];
            set_log_descriptor(log_file_fd);
            set_time(&init_time);

            // Write to log after restoring the file descriptor the information
            // received
            if (write_log_array("RECV_PIPE", std, 3) ||
                write_log_timeval("RECV_PIPE", init_time)) {
                write(STDERR_FILENO, "error upon writing log\n", 23);
            }

            subprocess = 1;
        } else {
            log_file_fd = init_log();
            set_time(&init_time);
        }

        // Write commands passed as arguments
        char buffer[BUFFER_SIZE];
        for (int i = 0; i < argc; i++) {
            strncat(buffer, argv[i], sizeof(char) * strlen(argv[i]));
            strncat(buffer, " ", 1);
        }
        strncat(buffer, "\n", 1);
        if (write_log("CREATE", buffer)) {
            write(STDERR_FILENO, "error upon writing log\n", 23);
        }
    }

    // Structs para dar handle aos sinais
    struct sigaction action, actionLog;

    // Struct para dar handle a SIGIN
    action.sa_handler = subprocess ? SIG_IGN : sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // Struct para dar handle a SIGTERM
    actionLog.sa_handler = siglog_handler;
    sigemptyset(&actionLog.sa_mask);
    actionLog.sa_flags = 0;

    // Instalação dos sigint_handler

    if (sigaction(SIGINT, &action, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGINT handler\n");
        exit(1);
    }
    if (sigaction(SIGTERM, &actionLog, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGTERM handler\n");
        exit(1);
    }
    if (sigaction(SIGCONT, &actionLog, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGCONT handler\n");
        exit(1);
    }
    // resetHandler(actionLog);

    // error | path | max-depth | S | L | B | b | a | l
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        exit_status = -1;
        return exit_status;
    }

    char *path;
    int block_size;
    int max_depth;

    block_size = (flags & FLAG_BSIZE) ? info.block_size : 1024;
    max_depth = (flags & FLAG_MAXDEPTH) ? info.max_depth - subprocess : -1;

    struct stat status;

    for (int path_index = 0; path_index < info.paths_size; path_index++) {
        path = info.paths[path_index];

        if (fget_status(path, &status, flags & FLAG_DEREF)) {
            free_parse_info(&info);
            exit_status = -1;
            return exit_status;
        }

        file_type_t ftype = sget_type(&status);
        double fsize = fget_size(flags & FLAG_BYTES, &status, block_size);

        switch (ftype) {
            case FTYPE_REG: {
                char buffer[BUFFER_SIZE];
                sprintf(buffer,
                        "%ld"
                        "\x9"
                        "%s\n",
                        dceill(fsize), path);
                write_log("ENTRY", buffer);
                write(STDOUT_FILENO, buffer, strlen(buffer));
            } break;
            case FTYPE_DIR: {
                DIR *dir;

                if ((dir = opendir(path)) == NULL) {
                    exit_status = error_sys("opendir error");
                    return exit_status;
                }

                struct dirent *direntp;

                while ((direntp = readdir(dir)) != NULL) {
                    // Skip . and .. directories
                    if (strcmp(direntp->d_name, ".") == 0 ||
                        strcmp(direntp->d_name, "..") == 0)
                        continue;

                    // Build new path
                    char new_path[BUFFER_SIZE];
                    sprintf(new_path, "%s%s%s", path,
                            ((path[strlen(path) - 1] == '/') ? "" : "/"),
                            direntp->d_name);

                    struct stat new_status;

                    if (fget_status(new_path, &new_status,
                                    flags & FLAG_DEREF)) {
                        exit_status = error_sys(
                            "fget_status error on reading directory's file "
                            "status");
                        return exit_status;
                    }

                    file_type_t new_type = sget_type(&new_status);
                    double new_fsize;
                    switch (new_type) {
                        case FTYPE_REG:
                            new_fsize = fget_size(flags & FLAG_BYTES,
                                                  &new_status, block_size);
                            fsize += new_fsize;
                            if ((flags & FLAG_ALL) &&
                                ((flags & FLAG_MAXDEPTH) == 0 ||
                                 max_depth > 0)) {
                                char buffer[BUFFER_SIZE];
                                sprintf(buffer,
                                        "%ld"
                                        "\x9"
                                        "%s\n",
                                        dceill(new_fsize), new_path);
                                if (write_log("ENTRY", buffer)) {
                                    write(STDERR_FILENO,
                                          "error upon writing log\n", 23);
                                }
                                write(STDOUT_FILENO, buffer, strlen(buffer));
                            }
                            break;
                        case FTYPE_DIR: {
                            // Build command line arguments
                            parse_info_t new_info;
                            init_parse_info(&new_info);
                            parse_info_addpath(&new_info, new_path);
                            new_info.block_size = block_size;
                            new_info.max_depth =
                                (max_depth > 0) ? max_depth : 0;

                            char **new_argv =
                                build_argv(argv[0], flags, &new_info);

                            free_parse_info(&new_info);

                            int pipe_ctosp[2];  // Pipe child to subprocess
                            int pipe_ctop[2];   // Pipe child to parent

                            int return_status;

                            if (pipe(pipe_ctosp) || pipe(pipe_ctop)) {
                                exit_status = error_sys("pipe error");
                                return exit_status;
                            }

                            pid_t pid = fork();

                            // sleep(2);

                            switch (pid) {
                                case -1:
                                    exit_status = error_sys("fork error");
                                    return exit_status;
                                case 0:  // Filho
                                {
                                    int std[3];
                                    if (!subprocess) {
                                        setpgid(0, 0);
                                    }
                                    if ((std[READ_PIPE] = dup(STDIN_FILENO)) ==
                                            -1 ||
                                        (std[WRITE_PIPE] =
                                             dup(STDOUT_FILENO)) == -1) {
                                        exit_status = error_sys(
                                            "dup error upon copying stdin and "
                                            "stdout descriptors");
                                        return exit_status;
                                    }
                                    if ((std[LOG_FILE] = dup(log_file_fd)) ==
                                        -1) {
                                        exit_status = error_sys(
                                            "dup error upon copying "
                                            "log_file_fd");
                                        return exit_status;
                                    }
                                    if (write(pipe_ctosp[WRITE_PIPE], std,
                                              sizeof(int) * 3) == -1) {
                                        exit_status = error_sys(
                                            "write error to subprocess "
                                            "connection pipe");
                                        return exit_status;
                                    }
                                    // write timeval to pipe
                                    if (write(pipe_ctosp[WRITE_PIPE],
                                              &init_time,
                                              sizeof(init_time)) == -1) {
                                        exit_status = error_sys(
                                            "write error to subprocess "
                                            "connection pipe");
                                        return exit_status;
                                    }
                                    // write log  of std
                                    if (write_log_array("SEND_PIPE", std, 3) ||
                                        write_log_timeval("SEND_PIPE",
                                                          init_time)) {
                                        write(STDERR_FILENO,
                                              "error upon writing log\n", 23);
                                    }
                                    if (close(pipe_ctop[READ_PIPE]) ||
                                        close(pipe_ctosp[WRITE_PIPE])) {
                                        exit_status = error_sys(
                                            "close error upon closing pipe");
                                        return exit_status;
                                    }

                                    if (dup2(pipe_ctop[WRITE_PIPE],
                                             STDOUT_FILENO) == -1 ||
                                        dup2(pipe_ctosp[READ_PIPE],
                                             STDIN_FILENO) == -1) {
                                        exit_status = error_sys(
                                            "dup2 error upon redefining "
                                            "descriptors pointed by "
                                            "stdin and stdout");
                                        return exit_status;
                                    }

                                    if (close(pipe_ctop[WRITE_PIPE]) ||
                                        close(pipe_ctosp[READ_PIPE])) {
                                        exit_status = error_sys(
                                            "close error upon closing pipe");
                                        return exit_status;
                                    }

                                    if (execv(argv[0], new_argv) == -1) {
                                        exit_status = error_sys("execv error");
                                        return exit_status;
                                    }
                                } break;
                                default:  // Pai
                                {
                                    setGlobalProcess(pid);

                                    if (close(pipe_ctop[WRITE_PIPE]) ||
                                        close(pipe_ctosp[WRITE_PIPE]) ||
                                        close(pipe_ctosp[READ_PIPE])) {
                                        exit_status = error_sys(
                                            "close error upon closing pipe");
                                        return exit_status;
                                    }

                                    do {
                                        if (waitpid(pid, &return_status, 0) ==
                                            -1) {
                                            if (errno == EINTR) continue;
                                            exit_status =
                                                error_sys("waitpid error");
                                            return exit_status;
                                        }
                                        break;
                                    } while (1);
                                    resetGlobalProcess();

                                    int i = 0;
                                    while (new_argv[i] != NULL) {
                                        free(new_argv[i]);
                                        i++;
                                    }
                                    free(new_argv);

                                    if (WIFEXITED(return_status) &&
                                        WEXITSTATUS(return_status) == 0) {
                                        double subdir_size = 0;
                                        if (read(pipe_ctop[READ_PIPE],
                                                 &subdir_size,
                                                 sizeof(double)) == -1) {
                                            exit_status = error_sys(
                                                "write error upon reading from "
                                                "child connection "
                                                "pipe");
                                            return exit_status;
                                        }
                                        if (write_log_double("RECV_PIPE",
                                                             subdir_size)) {
                                            write(STDERR_FILENO,
                                                  "error upon writing log\n",
                                                  23);
                                        }

                                        fsize += (flags & FLAG_SEPDIR)
                                                     ? 0
                                                     : subdir_size;

                                        if (close(pipe_ctop[READ_PIPE])) {
                                            exit_status = error_sys(
                                                "close error upon closing "
                                                "pipe");
                                            return exit_status;
                                        }
                                    }

                                } break;
                            }
                        } break;
                        case FTYPE_LINK: {
                            new_fsize = fget_size(flags & FLAG_BYTES,
                                                  &new_status, block_size);
                            fsize += new_fsize;
                            if ((flags & FLAG_ALL) &&
                                ((flags & FLAG_MAXDEPTH) == 0 ||
                                 max_depth > 0)) {
                                char buffer[BUFFER_SIZE];
                                sprintf(buffer,
                                        "%ld"
                                        "\x9"
                                        "%s\n",
                                        dceill(new_fsize), new_path);
                                if (write_log("ENTRY", buffer)) {
                                    write(STDERR_FILENO,
                                          "error upon writing log\n", 23);
                                }
                                write(STDOUT_FILENO, buffer, strlen(buffer));
                            }
                        } break;
                        default:
                            break;
                    }
                }
                if (!subprocess || (flags & FLAG_MAXDEPTH) == 0 ||
                    max_depth >= 0) {
                    char buffer[BUFFER_SIZE];
                    sprintf(buffer,
                            "%ld"
                            "\x9"
                            "%s\n",
                            dceill(fsize), path);
                    if (write_log("ENTRY", buffer)) {
                        write(STDERR_FILENO, "error upon writing log\n", 23);
                    }
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                }

                if (subprocess) {
                    if (write(ppipe_write, &fsize, sizeof(double)) == -1) {
                        exit_status = error_sys(
                            "write error upong writing to parent connection "
                            "pipe and/or "
                            "write information received by pipe to log");
                        return exit_status;
                    }
                }

                if (write_log_double("SEND_PIPE", fsize)) {
                    write(STDERR_FILENO, "error upon writing log\n", 23);
                }

                if (closedir(dir)) {
                    exit_status = error_sys("closedir");
                    return exit_status;
                }
            } break;
            case FTYPE_LINK: {
                // Dereference symbolic link if flag is set
                char buffer[BUFFER_SIZE];
                sprintf(buffer,
                        "%ld"
                        "\x9"
                        "%s\n",
                        dceill(fsize), path);
                if (write_log("ENTRY", buffer)) {
                    write(STDOUT_FILENO, "error upon writing log", 22);
                }
                write(STDOUT_FILENO, buffer, strlen(buffer));
            } break;
            default:
                break;
        }
    }

    if (subprocess) {
        if (close(ppipe_write)) {
            exit_status = error_sys("close error upon closing pipe");
            return exit_status;
        }
    }

    // free memory
    free_parse_info(&info);

    // Close log file
    /*if(subprocess == 0) {
        close_log();
    }*/
    exit_status = 0;
    return 0;
}
