/* MAIN HEADER */

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/* Miscellaneous */

char NAME_SEM_SEND_REQUEST[] = "/sem_send_request";
char NAME_SEM_RECEIVE_REQUEST[] = "/sem_receive_request";
char SEM_PREFIX[] = "/sem_";

sem_t *sem_send_request;
sem_t *sem_receive_request;

/*----------------------------------------------------------------------------*/
/*                              REQUEST SEMAPHORES                            */
/*----------------------------------------------------------------------------*/

int init_sync(int oflags) {
    if ((sem_send_request = sem_open(NAME_SEM_SEND_REQUEST, oflags, 0600, 0)) == SEM_FAILED) {
        return errno;
    }

    if ((sem_receive_request = sem_open(NAME_SEM_RECEIVE_REQUEST, oflags, 0600, 0)) == SEM_FAILED) {
        return errno;
    }

    return 0;
}

int free_sync() {
    if (sem_send_request == NULL && sem_receive_request == NULL) return 0;

    int ret = 0;

    if (sem_send_request != NULL) {
        if (sem_close(sem_send_request)) {
            ret = errno;
        }

        if (sem_unlink(NAME_SEM_SEND_REQUEST)) {
            ret = errno;
        }
    }

    if (sem_receive_request != NULL) {
        if (sem_close(sem_receive_request)) {
            ret = errno;
        }

        if (sem_unlink(NAME_SEM_RECEIVE_REQUEST)) {
            ret = errno;
        }
    }
    return ret;
}

int sem_wait_send_request() {
    if (sem_wait(sem_send_request)) {
        return errno;
    }
    return 0;
}

int sem_post_send_request() {
    if (sem_post(sem_send_request)) {
        return errno;
    }
    return 0;
}

int sem_getvalue_send_request(int *val) {
    if (val == NULL) return -1;
    if (sem_getvalue(sem_send_request, val)) {
        return -1;
    }
    return 0;
}

int sem_wait_receive_request() {
    if (sem_wait(sem_receive_request)) {
        return errno;
    }
    return 0;
}

int sem_post_receive_request() {
    if (sem_post(sem_receive_request)) {
        return errno;
    }
    return 0;
}

int sem_getvalue_receive_request(int *val) {
    if (val == NULL) return -1;
    if (sem_getvalue(sem_receive_request, val)) {
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/*                              REPLY SEMAPHORE                               */
/*----------------------------------------------------------------------------*/

sem_t* sem_open_reply(pid_t pid, pthread_t tid) {
    char sem_name[256];
    sprintf(sem_name, "%s%d.%ld", SEM_PREFIX, pid, tid);

    sem_t *sem_reply;

    if ((sem_reply = sem_open(sem_name, O_CREAT, 0600, 0)) == SEM_FAILED) {
        return NULL;
    }

    return sem_reply;
}

int sem_wait_reply(sem_t *sem_reply) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    if (sem_wait(sem_reply)) {
        return errno;
    }
    return 0;
}

int sem_post_reply(sem_t *sem_reply) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    if (sem_post(sem_reply)) {
        return errno;
    }
    return 0;
}

int sem_close_reply(sem_t *sem_reply) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    if (sem_close(sem_reply)) {
        return errno;
    }

    return 0;
}

int sem_unlink_reply(const char *sem_name) {
    if (sem_name == NULL) {
        errno = EINVAL;
        return errno;
    }

    if (sem_unlink(sem_name)) {
        return errno;
    }

    return 0;
}

int sem_free_reply(sem_t *sem_reply, pid_t pid, pthread_t tid) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    char sem_name[256];
    sprintf(sem_name, "%s%d.%ld", SEM_PREFIX, pid, tid);

    if (sem_close_reply(sem_reply)) {
        return errno;
    }

    if (sem_unlink_reply(sem_name)) {
        return errno;
    }

    return 0;
}
