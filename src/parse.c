/* MAIN HEADER */
#include "parse.h"

/* INCLUDE HEADERS */
#include "utils.h"

/* SYSTEM CALLS  HEADERS */
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_parse_info(parse_info_t *info) {
    info->path = NULL;
    info->block_size = 0;
    info->max_depth = 0;
}

char** build_argv(char *argv0, int flags, parse_info_t *info) {

    int n = 1; // argv size initialized with 1 for argv0
    for (int i = 0, k = 1; i < 7; i++, k <<= 1) { // ignore path flag
        n += ((flags & k) != 0); // add space for each flag activated
    }
    n = n + 2; // add space for path and for null pointer
    char **cmd = (char**)malloc(sizeof(char*) * n);

    cmd[0] = strdup(argv0);
    int i = 1;
    if (flags & FLAG_LINKS) {
        cmd[i++] = strdup("--count-links");
    }
    if (flags & FLAG_ALL) {
        cmd[i++] = strdup("--all");
    }
    if (flags & FLAG_BYTES) {
        cmd[i++] = strdup("--bytes");
    }
    if (flags & FLAG_BSIZE) {
        char num[50];
        sprintf(num, "%d", info->block_size);
        cmd[i++] = str_cat("--block-size=", num, strlen(num));
    }
    if (flags & FLAG_DEREF) {
        cmd[i++] = strdup("--dereference");
    }
    if (flags & FLAG_SEPDIR) {
        cmd[i++] = strdup("--separate-dirs");
    }
    if (flags & FLAG_MAXDEPTH) {
        char num[50];
        sprintf(num, "%d", info->max_depth);
        cmd[i++] = str_cat("--max-depth=", num, strlen(num));
    }
    cmd[i++] = strdup(info->path);
    cmd[i] = NULL;

    return cmd;
}

int parse_cmd(int argc, char *argv[], parse_info_t *info) {
    int flags = 0;

    if (argv == NULL || info == NULL) {
        write(STDERR_FILENO, "Invalid argument, one or more null pointers\n", 43);
        flags |= FLAG_ERR;
        return flags;
    }

    // Obligatory flag -l
    if (strcmp(argv[0], "-l") != 0 && strcmp(argv[0], "--count-links") != 0) {
        write(STDERR_FILENO, "Missing obligatory flag: -l\n", 27);
        flags |= FLAG_ERR;
        return flags;
    }

    flags |= FLAG_LINKS;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            if (flags & FLAG_ALL) {
                write(STDERR_FILENO, "Repeated flag: -a or --all\n", 27);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_ALL; // update flag
        }
        else if (strcmp(argv[i], "--bytes") == 0) {
            if (flags & FLAG_BYTES) {
                write(STDERR_FILENO, "Repeated flag: -b or --bytes\n", 29);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_BYTES; // update flag
        }
        else if (strncmp(argv[i], "--block-size=", 13) == 0) {
            if (flags & FLAG_BSIZE) {
                write(STDERR_FILENO, "Repeated flag: -B or --block-size\n", 36);
                flags |= FLAG_ERR;
                return flags;
            }

            char *tmp = argv[i] + 13; // skip "--block-size="

            if (strlen(tmp) == 0 || str_isDigit(tmp) < 1) {
                write(STDERR_FILENO, "Flag -B or --block-size missing an integer\n", 43);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(tmp, "%d", &(info->block_size));

            flags |= FLAG_BSIZE; // update flag
        }
        else if (strcmp(argv[i], "--dereference") == 0) {
            if (flags & FLAG_DEREF) {
                write(STDERR_FILENO, "Repeated flag: -L or --dereference\n", 35);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_DEREF; // update flag
        }
        else if (strcmp(argv[i], "--separate-dirs") == 0) {
            if (flags & FLAG_SEPDIR) {
                write(STDERR_FILENO, "Repeated flag: -S or --separate-dirs\n", 37);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_SEPDIR; // update flag
        }
        else if (strncmp(argv[i], "--max-depth=", 12) == 0) {
            if (flags & FLAG_MAXDEPTH) {
                write(STDERR_FILENO, "Repeated flag: --max-depth\n", 39);
                flags |= FLAG_ERR;
                return flags;
            }

            char *tmp = argv[i] + 12; // skip "--max-depth="

            if (strlen(tmp) == 0 || str_isDigit(tmp) < 1) {
                write(STDERR_FILENO, "Flag --max-depth must have an integer\n", 46);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(tmp, "%d", &(info->max_depth));

            flags |= FLAG_MAXDEPTH; // update flag
        }
        else if (strncmp(argv[i], "-", 1) == 0) {
            char *tmp = argv[i] + 1; // skip "-"

            if (strlen(tmp) == 0 || str_isAlpha(tmp) < 1) {
                write(STDERR_FILENO, "Invalid flag or flags\n", 24);
                flags |= FLAG_ERR;
                return flags;
            }

            if (str_find(tmp, "l", 0) >= 0) {
                if (flags & FLAG_ALL) {
                    write(STDERR_FILENO, "Repeated flag: -a or --all\n", 27);
                    flags |= FLAG_ERR;
                    return flags;
                }
            }
            if (str_find(tmp, "a", 0) >= 0) {
                if (flags & FLAG_ALL) {
                    write(STDERR_FILENO, "Repeated flag: -a or --all\n", 27);
                    flags |= FLAG_ERR;
                    return flags;
                }

                flags |= FLAG_ALL; // update flag
            }
            if (str_find(tmp, "b", 0) >= 0) {
                if (flags & FLAG_BYTES) {
                    write(STDERR_FILENO, "Repeated flag: -b or --bytes\n", 29);
                    flags |= FLAG_ERR;
                    return flags;
                }

                flags |= FLAG_BYTES; // update flag
            }
            if (str_find(tmp, "L", 0) >= 0) {
                if (flags & FLAG_DEREF) {
                    write(STDERR_FILENO, "Repeated flag: -L or --dereference\n", 35);
                    flags |= FLAG_ERR;
                    return flags;
                }

                flags |= FLAG_DEREF; // update flag
            }
            if (str_find(tmp, "S", 0) >= 0) {
                if (flags & FLAG_SEPDIR) {
                    write(STDERR_FILENO, "Repeated flag: -S or --separate-dirs\n", 37);
                    flags |= FLAG_ERR;
                    return flags;
                }

                flags |= FLAG_SEPDIR; // update flag
            }
            if (str_find(tmp, "B", 0) >= 0) {
                if (flags & FLAG_BSIZE) {
                    write(STDERR_FILENO, "Repeated flag: -B or --block-size\n", 34);
                    flags |= FLAG_ERR;
                    return flags;
                }

                if (i + 1 >= argc || strlen(argv[i+1]) == 0 || str_isDigit(argv[i+1]) < 1) {
                    write(STDERR_FILENO, "Flag -B or --block-size missing an integer\n", 43);
                    flags |= FLAG_ERR;
                    return flags;
                }

                sscanf(argv[i+1], "%d", &(info->block_size));

                flags |= FLAG_BSIZE; // update flag
                i++;
            }
        }
        else { // verify if it's valid path

            if (flags & FLAG_PATH) {
                write(STDERR_FILENO, "Repeated flag: path\n", 20);
                flags |= FLAG_ERR;
                return flags;
            }

            struct stat status;

            if (fget_status(argv[i], &status, 0) == -1) {
                flags |= FLAG_ERR;
                return flags;
            }

            info->path = strdup(argv[i]);
            if (rtrim(info->path, '/', MODE_RMDUP)) {
                write(STDERR_FILENO, "Error trimming path\n", 20);
                flags |= FLAG_ERR;
                return flags;
            }
            flags |= FLAG_PATH;
        }
    }



    return flags;
}
