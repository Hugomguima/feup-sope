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
    info->exec_secs = -1;
}

void free_parse_info(parse_info_t *info) {
    if (info == NULL) return;
    if (info->path != NULL) free(info->path);
}

int parse_cmd(int argc, char *argv[], parse_info_t *info) {
    int flags = 0;

    if (argv == NULL || info == NULL) {
        write(STDERR_FILENO, "Invalid argument, one or more null pointers\n", 44);
        flags |= FLAG_ERR;
        return flags;
    }

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 >= argc || strlen(argv[i + 1]) == 0 || str_isDigit(argv[i + 1]) < 1) {
                write(STDERR_FILENO, "Flag -t missing an integer\n", 27);
                flags |= FLAG_ERR;
                return flags;
            }

            sscanf(argv[i + 1], "%d", &(info->exec_secs));
            flags |= FLAG_SECS;  // execution duration flag
            i++;
        } else {
            if (info->path != NULL) free(info->path);
            info->path = strdup(argv[i]);
            flags |= FLAG_FIFO;
        }
    }

    if ((flags & FLAG_SECS) == 0 || (flags & FLAG_FIFO) == 0) {
        write(STDERR_FILENO, "Missing obligatory flag -t or FIFO name\n", 40);
        flags |= FLAG_ERR;
        return flags;
    }
    return flags;
}
