/* MAIN HEADER */
#include "protocol.h"

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Miscellaneous */
#include <pthread.h>

int fill_request(request_t *request, int id, pid_t pid, pthread_t tid) {
    if (request == NULL) {
        errno = EINVAL;
        return errno;
    }

    request->id = id;
    request->pid = pid;
    request->tid = tid;
    request->dur = MIN_DURATION + random() % (MAX_DURATION - MIN_DURATION);
    request->pl = -1;

    return 0;
}

int write_request(int fd, const request_t *request) {
    if (write(fd, request, sizeof(request_t)) != sizeof(request_t)) {
        return errno;
    }
    return 0;
}

int read_request(int fd, request_t *request) {
    if (read(fd, request, sizeof(request_t)) != sizeof(request_t)) {
        return errno;
    }
    return 0;
}

int write_reply(int fd, const request_t *reply) {
    if (write(fd, reply, sizeof(request_t)) != sizeof(request_t)) {
        return errno;
    }
    return 0;
}

int read_reply(int fd, request_t *reply) {
    if (read(fd, reply, sizeof(request_t)) != sizeof(request_t)) {
        return errno;
    }
    return 0;
}

int fill_reply(request_t *reply, int id, pid_t pid, pthread_t tid, int dur, int pl) {
    if (reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    reply->id = id;
    reply->pid = pid;
    reply->tid = tid;
    reply->dur = dur;
    reply->pl = pl;

    return 0;
}

int fill_reply_error(request_t *reply, int id, pid_t pid, pthread_t tid) {
    if (reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    reply->id = id;
    reply->pid = pid;
    reply->tid = tid;
    reply->dur = -1;
    reply->pl = -1;

    return 0;
}
