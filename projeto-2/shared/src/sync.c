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
        perror("open send request semaphore");
        return errno;
    }

    if ((sem_receive_request = sem_open(NAME_SEM_RECEIVE_REQUEST, oflags, 0600, 0)) == SEM_FAILED) {
        perror("open receive request semaphore");
        return errno;
    }

    return 0;
}

int free_sync() {
    if (sem_send_request == NULL && sem_receive_request == NULL) return 0;

    int ret = 0;

    if (sem_send_request != NULL) {
        if (sem_close(sem_send_request)) {
            perror("close send request semaphore");
            ret = errno;
        }

        if (sem_unlink(NAME_SEM_SEND_REQUEST)) {
            perror("unlink send request semaphore");
            ret = errno;
        }
    }

    if (sem_receive_request != NULL) {
        if (sem_close(sem_receive_request)) {
            perror("close receive request semaphore");
            ret = errno;
        }

        if (sem_unlink(NAME_SEM_RECEIVE_REQUEST)) {
            perror("unlink receive request semaphore");
            ret = errno;
        }
    }
    return ret;
}

int sem_wait_send_request() {
    if (sem_wait(sem_send_request)) {
        perror("wait send request semaphore");
        return errno;
    }
    return 0;
}

int sem_post_send_request() {
    if (sem_post(sem_send_request)) {
        perror("wait send request semaphore");
        return errno;
    }
    return 0;
}

int sem_wait_receive_request() {
    if (sem_wait(sem_receive_request)) {
        perror("wait receive request semaphore");
        return errno;
    }
    return 0;
}

int sem_post_receive_request() {
    if (sem_post(sem_receive_request)) {
        perror("wait receive request semaphore");
        return errno;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/*                              REPLY SEMAPHORE                               */
/*----------------------------------------------------------------------------*/

int sem_open_reply(sem_t *sem_reply, pid_t pid, pthread_t tid) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    char sem_name[256];
    sprintf(sem_name, "%s%d.%ld", SEM_PREFIX, pid, tid);

    if ((sem_reply = sem_open(sem_name, O_CREAT, 0600, 0)) == SEM_FAILED) {
        perror("open reply semaphore");
        return errno;
    }

    return 0;
}

int sem_wait_reply(sem_t *sem_reply) {
    if (sem_reply == NULL) {
        errno = EINVAL;
        return errno;
    }

    if (sem_wait(sem_reply)) {
        perror("wait reply semaphore");
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
        perror("post reply semaphore");
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
        perror("close reply semaphore");
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
        perror("unlink reply semaphore");
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
