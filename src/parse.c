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
    info->paths = NULL;
    info->paths_size = 0;
    info->paths_memsize = 0;
    info->block_size = 0;
    info->max_depth = 0;
}

void free_parse_info(parse_info_t *info) {
    if (info == NULL) return;
    for (int i = 0; i < info->paths_size; i++) {
        if (info->paths[i] != NULL) free(info->paths[i]);
    }
}

void parse_info_addpath(parse_info_t *info, char *path) {
    if (info->paths_size == info->paths_memsize) {
        info->paths_memsize *= 2;
        info->paths = (char**)realloc(info->paths, sizeof(char*) * info->paths_memsize);
    }

    info->paths[info->paths_size++] = strdup(path);
}

char** build_argv(char *argv0, int flags, parse_info_t *info) {

    int n = 1; // argv size initialized with 1 for argv0
    for (int i = 0, k = 1; i < 7; i++, k <<= 1) { // ignore path flag
        n += ((flags & k) != 0); // add space for each flag activated
    }
    n = n + info->paths_size; // add space for paths
    n = n + 1; // add space for null pointer
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
    for (int j = 0; j < info->paths_size; j++) {
        cmd[i++] = strdup(info->paths[j]);
    }
    cmd[i] = NULL;

    return cmd;
}

int parse_cmd(int argc, char *argv[], parse_info_t *info) {
    int flags = 0;

    if (argv == NULL || info == NULL) {
        write(STDERR_FILENO, "Invalid argument, one or more null pointers\n", 44);
        flags |= FLAG_ERR;
        return flags;
    }

    info->paths_memsize = 1;
    info->paths = (char**)malloc(sizeof(char*) * info->paths_memsize);

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            flags |= FLAG_ALL; // update flag
        }
        else if (strcmp(argv[i], "--bytes") == 0) {
            flags &= ~FLAG_BSIZE; // remove flag
            flags |= FLAG_BYTES; // update flag
        }
        else if (strcmp(argv[0], "--count-links") == 0) {
            flags |= FLAG_LINKS;
        }
        else if (strncmp(argv[i], "--block-size=", 13) == 0) {
            char *tmp = argv[i] + 13; // skip "--block-size="

            if (strlen(tmp) == 0 || str_isDigit(tmp) < 1) {
                write(STDERR_FILENO, "Flag -B or --block-size missing an integer\n", 43);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(tmp, "%d", &(info->block_size));

            flags &= ~FLAG_BYTES;
            flags |= FLAG_BSIZE; // update flag
        }
        else if (strcmp(argv[i], "--dereference") == 0) {
            flags |= FLAG_DEREF; // update flag
        }
        else if (strcmp(argv[i], "--separate-dirs") == 0) {
            flags |= FLAG_SEPDIR; // update flag
        }
        else if (strncmp(argv[i], "--max-depth=", 12) == 0) {
            char *tmp = argv[i] + 12; // skip "--max-depth="

            if (strlen(tmp) == 0 || str_isDigit(tmp) < 1) {
                write(STDERR_FILENO, "Flag --max-depth must have an integer\n", 38);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(tmp, "%d", &(info->max_depth));

            flags |= FLAG_MAXDEPTH; // update flag
        }
        else if (strncmp(argv[i], "-", 1) == 0) {
            char *tmp = argv[i] + 1; // skip "-"

            if (strlen(tmp) == 0) { // verify if it's valid path

            struct stat status;

            if (fget_status(argv[i], &status, 0) == -1) {
                flags |= FLAG_ERR;
                return flags;
            }

            parse_info_addpath(info, argv[i]);
            if (rtrim(info->paths[info->paths_size - 1], '/', MODE_RMDUP)) {
                write(STDERR_FILENO, "Error trimming path\n", 20);
                flags |= FLAG_ERR;
                return flags;
            }
            flags |= FLAG_PATH;

        }

        if (str_isAlpha(tmp) < 1) {
            write(STDERR_FILENO, "Invalid flag or flags\n", 22);
            flags |= FLAG_ERR;
            return flags;
        }

        if (str_find(tmp, "l", 0) >= 0) {
            flags |= FLAG_LINKS;
        }
        if (str_find(tmp, "a", 0) >= 0) {
            flags |= FLAG_ALL; // update flag
        }
        if (str_find(tmp, "b", 0) >= 0) {
            flags &= ~FLAG_BSIZE;
            flags |= FLAG_BYTES; // update flag
        }
        if (str_find(tmp, "L", 0) >= 0) {
            flags |= FLAG_DEREF; // update flag
        }
        if (str_find(tmp, "S", 0) >= 0) {
            flags |= FLAG_SEPDIR; // update flag
        }
        if (str_find(tmp, "B", 0) >= 0) {

            if (i + 1 >= argc || strlen(argv[i+1]) == 0 || str_isDigit(argv[i+1]) < 1) {
                write(STDERR_FILENO, "Flag -B or --block-size missing an integer\n", 43);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(argv[i+1], "%d", &(info->block_size));

            flags &= ~FLAG_BYTES;
            flags |= FLAG_BSIZE; // update flag
            i++;
        }
    }
    else { // verify if it's valid path
    struct stat status;

    if (fget_status(argv[i], &status, 0) == -1) {
        flags |= FLAG_ERR;
        return flags;
    }

    parse_info_addpath(info, argv[i]);
    if (rtrim(info->paths[info->paths_size - 1], '/', MODE_RMDUP)) {
        write(STDERR_FILENO, "Error trimming path\n", 20);
        flags |= FLAG_ERR;
        return flags;
    }
    flags |= FLAG_PATH;
}
}

// Obligatory flag -l
if ((flags & FLAG_LINKS) == 0) {
    write(STDERR_FILENO, "Missing obligatory flag: -l or --count-links\n", 45);
    flags |= FLAG_ERR;
    return flags;
}

if (info->paths_size == 0) {
    parse_info_addpath(info, ".");
}



return flags;
}
