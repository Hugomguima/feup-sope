#include <parse.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <utils.h>

int parse_cmd(int argc, char *argv[]) {
    char flags = 0;

    // Obligatory flag -l
    if (strcmp(argv[0], "-l") != 0 && strcmp(argv[0], "--count-links") != 0) {
        write(STDERR_FILENO, "Missing obligatory flag: -l\n", 28);
        flags |= FLAG_ERR;
        return flags;
    }

    flags |= FLAG_LINKS;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            if (flags & FLAG_ALL) {
                write(STDERR_FILENO, "Repeated flag: -a or --all\n", 28);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_ALL; // update flag
        }
        else if (strcmp(argv[i], "--bytes") == 0) {
            if (flags & FLAG_BYTES) {
                write(STDERR_FILENO, "Repeated flag: -b or --bytes\n", 30);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_BYTES; // update flag
        }
        else if (strncmp(argv[i], "--block-size=", 13) == 0) {
            if (flags & FLAG_BSIZE) {
                write(STDERR_FILENO, "Repeated flag: -B or --block-size\n", 35);
                flags |= FLAG_ERR;
                return flags;
            }

            char *tmp = argv[i] + 13; // skip "--block-size="

            if (!str_isDigit(tmp)) {
                write(STDERR_FILENO, "Flag -B or --block-size must have an integer\n", 47);
                flags |= FLAG_ERR;
                return flags;
            }

            int size;

            sscanf(tmp, "%d", &size);
            printf("Size: %d\n", size);

            flags |= FLAG_BYTES; // update flag
        }
        else if (strcmp(argv[i], "--dereference") == 0) {
            if (flags & FLAG_DEREF) {
                write(STDERR_FILENO, "Repeated flag: -L or --dereference\n", 36);
                flags |= FLAG_ERR;
                return flags;
            }

            flags |= FLAG_DEREF; // update flag
        }
        else if (strncmp(argv[i], "--max-depth=", 12) == 0) {
            if (flags & FLAG_MAXDEPTH) {
                write(STDERR_FILENO, "Repeated flag: --max-depth\n", 38);
                flags |= FLAG_ERR;
                return flags;
            }

            char *tmp = argv[i] + 12; // skip "--max-depth="

            if (!str_isDigit(tmp)) {
                write(STDERR_FILENO, "Flag --max-depth must have an integer\n", 47);
                flags |= FLAG_ERR;
                return flags;
            }

            int depth;

            sscanf(tmp, "%d", &depth);
            printf("Max Depth: %d\n", depth);

            flags |= FLAG_MAXDEPTH; // update flag
        }
    }



    return flags;
}
