#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

#define BUFFER_SIZE 255

int main(int argc, char *argv[]) {

    if (argc < 2 || argc > 9) {

        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    return 0;
}
