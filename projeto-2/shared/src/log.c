/* MAIN HEADER */
#include "log.h"

/* INCLUDE HEADERS */
#include "protocol.h"

/* SYSTEM CALLS HEADERS */

/* C LIBRARY HEADERS */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int write_log(request_t *message, char *operation) {
    if (message == NULL || operation == NULL) {
        errno = EINVAL;
        return -1;
    }
    char buf[256];
    sprintf(buf, "%ld \t; %d \t; %d \t; %ld \t; %d \t; %d \t; %s\n", time(NULL), message->id, message->pid, message->tid, message->dur, message->pl, operation);
    if (write(STDOUT_FILENO, buf, strlen(buf)) != strlen(buf)) {
        return -1;
    }
    return 0;
}
