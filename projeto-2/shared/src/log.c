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

int write_log(request_t reply, char *message) {
    char buf[256];
    sprintf(buf, "%ld ; %d ; %d ; %ld ; %d ; %d ; %s \n", time(NULL), reply.id, reply.pid, reply.tid, reply.dur, reply.pl, message);
    if (write(STDOUT_FILENO, buf, strlen(buf)) != strlen(buf)) {
        return errno;
    }
    return 0;
}
