/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "constants.h"
#include "error.h"

/* SYSTEM CALLS HEADERS */
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

char NAME_SEM_SEND_REQUEST[] = "/sem_send_request";
char NAME_SEM_RECEIVE_REQUEST[] = "/sem_receive_request";
char SEM_PREFIX[] = "/sem_";

sem_t *sem_send_request;
sem_t *sem_receive_request;

/*----------------------------------------------------------------------------*/
/*                              ALARM                                         */
/*----------------------------------------------------------------------------*/
int alarm_counter = 0;
int stopped_counter = 0;

void stop_notification(int sig) { }

void *my_alarm(void *arg) {
    if (arg == NULL) {
        char program[BUFFER_SIZE];
        sprintf(program, "alarm %ld", pthread_self());
        error_sys(program, "invalid argument");
        return NULL;
    }
    void **args = (void**)arg;

    if (args[0] == NULL || args[1] == NULL || args[2] == NULL) {
        char program[BUFFER_SIZE];
        sprintf(program, "alarm %ld", pthread_self());
        error_sys(program, "invalid argument");
        if (args[0] != NULL) free(args[0]);
        if (args[1] != NULL) free(args[1]);
        if (args[2] != NULL) free(args[2]);
        free(arg);
        return NULL;
    }

    pthread_t tid = *((pthread_t*)args[0]);
    int exec_secs = *((int*)args[1]);
    int signal = *((int*)args[2]);

    struct sigaction alarm_deactivator;
    alarm_deactivator.sa_handler = stop_notification;
    sigemptyset(&alarm_deactivator.sa_mask);
    alarm_deactivator.sa_flags = 0;
    if (sigaction(SIGUSR2, &alarm_deactivator, NULL)) {
        char program[BUFFER_SIZE];
        sprintf(program, "alarm %ld", pthread_self());
        error_sys(program, "couldn't install alarm %ld deactivator");
        free(args[0]);
        free(args[1]);
        free(args[2]);
        free(arg);
        return NULL;
    }

    if (sleep(exec_secs) == 0) { // wasn't stopped
        if (pthread_kill(tid, signal)) {
            char program[BUFFER_SIZE];
            sprintf(program, "alarm %ld", pthread_self());
            char error[BUFFER_SIZE];
            sprintf(error, "couldn't notify thread %ld", tid);
            error_sys(program, error);
        }
    }
    free(args[0]);
    free(args[1]);
    free(args[2]);
    free(arg);
    return NULL;
}

int create_alarm(pthread_t tid, int exec_secs, pthread_t *ret) {
    void **args = malloc(sizeof(void*) * 3);
    args[0] = malloc(sizeof(pthread_t));
    args[1] = malloc(sizeof(int));
    args[2] = malloc(sizeof(int));
    *((pthread_t*)args[0]) = tid;
    *((int*)args[1]) = exec_secs;
    *((int*)args[2]) = ((ret == NULL) ? SIGALRM : SIGUSR1);

    pthread_t alarm_tid;

    if (pthread_create(&alarm_tid, NULL, my_alarm, args)) {
        error_sys("alarm", "couldn't create alarm thread");
        return -1;
    }

    if (pthread_detach(alarm_tid)) {
        char error[BUFFER_SIZE];
        sprintf(error, "couldn't detach alarm thread %ld", tid);
        error_sys("alarm", error);
        return -1;
    }
    if (ret != NULL) *ret = alarm_tid;
    return 0;
}

int stop_alarm(pthread_t tid) {
    return pthread_kill(tid, SIGUSR2);
}


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
