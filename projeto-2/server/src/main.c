/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "constants.h"
#include "error.h"
#include "log.h"
#include "parse.h"
#include "protocol.h"
#include "sync.h"
#include "utils.h"

/* SYSTEM CALLS HEADERS */
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Miscellaneous */
#include <pthread.h>

// TEMPORARY
#include <time.h>

int bath_open = 1;
int alarm_status = ALARM_CHILL;
int order = 1;
char req_fifo_path[BUFFER_SIZE];

void close_bathroom(int sig) {
    bath_open = 0;
    alarm_status = ALARM_TRIGGERED;
    chmod(req_fifo_path, 0440);
}

void *th_operation(void *arg);

void free_shared_memory() {
    if (free_sync()) {
        error_sys("exit", "couldn't free synchronization system");
    }
}

int request_queue_not_empty();

int main(int argc, char *argv[]) {

    if (argc < 4) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname", EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    // error | | | | fifoname | threads | places | secs
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        return -1;
    }

    sprintf(req_fifo_path, "/tmp/%s", info.path);

    int exec_secs = info.exec_secs;

    free_parse_info(&info);

    if (mkfifo(req_fifo_path, 0660)) {
        return error_sys(argv[0], "couldn't create public request FIFO");
    }

    int req_fifo;
    if ((req_fifo = open(req_fifo_path, O_RDONLY | O_NONBLOCK)) == -1) {
        unlink(req_fifo_path);
        return error_sys(argv[0], "couldn't open public request FIFO");
    }

    if (init_sync(O_CREAT)) {
        close(req_fifo);
        unlink(req_fifo_path);
        return error_sys(argv[0], "couldn't initialize synchronization system");
    }

    if (atexit(free_shared_memory)) {
        free_shared_memory();
        return error_sys(argv[0], "couldn't install atexit function");
    }

    struct sigaction alarm_action;
    alarm_action.sa_handler = close_bathroom;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        close(req_fifo);
        unlink(req_fifo_path);
        return error_sys(argv[0], "couldn't install alarm");
    }

    pthread_t tid;

    int thread_counter = 0;

    request_t *request;

    // DEV
    time_t initial = time(NULL);

    printf("%d\n", exec_secs);

    if (create_alarm(pthread_self(), exec_secs, SIGALRM, NULL)) {
        return error_sys(argv[0], "couldn't create alarm");
    }

    int not_empty = request_queue_not_empty();

    while (bath_open || not_empty) {
        if (not_empty == -1) {
            error_sys(argv[0], "error reading request queue");
            close(req_fifo);
            unlink(req_fifo_path);
            return errno;
        }

        if (sem_post_send_request()) {
            if (error_sys_ignore_alarm(argv[0], "couldn't unlock request queue", alarm_status)) {
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
            break;
        }
        if (sem_wait_receive_request()) {
            if (error_sys_ignore_alarm(argv[0], "error on waiting for request", alarm_status)) {
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
            break;
        }
        request = (request_t*)malloc(sizeof(request_t));

        if (read_request(req_fifo, request)) {
            if (error_sys_ignore_alarm(argv[0], "couldn't read request queue", alarm_status)) {
                free(request);
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
        }

        if(write_log(request, "RECVD")) {
            error_sys(argv[0], "couldn't write log");
        }

        if (pthread_create(&tid, NULL, th_operation, request)) {
            if (error_sys_ignore_alarm(argv[0], "couldn't create thread to attend request", alarm_status)) {
                free(request);
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
            if (pthread_create(&tid, NULL, th_operation, request)) {
                error_sys(argv[0], "couldn't create thread to attend request");
                free(request);
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
        }

        if (pthread_detach(tid)) {
            char error[BUFFER_SIZE];
            sprintf(error, "couldn't detach processing thread %ld", tid);
            if (error_sys_ignore_alarm(argv[0], error, alarm_status)) {
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
            if (pthread_detach(tid)) {
                close(req_fifo);
                unlink(req_fifo_path);
                return error_sys(argv[0], error);
            }
        }
        not_empty = request_queue_not_empty();
    }
    /*
    int empty;
    while (empty = request_queue_not_empty()) {
        if (empty == -1) {
            error_sys(argv[0], "couldn't ignore request");
            close(req_fifo);
            unlink(req_fifo_path);
            return errno;
        }

        request_t ignored_request;

        if (read_request(req_fifo, &ignored_request)) {
            error_sys(argv[0], "couldn't read request queue");
            close(req_fifo);
            unlink(req_fifo_path);
            return errno;
        }
        request_t error_reply;
        fill_reply_error(&error_reply, ignored_request.id, ignored_request.pid, ignored_request.tid);
        if(write_log(&error_reply, "2LATE")) {
            error_sys(argv[0], "couldn't write log");
        }

        if (sem_post_send_request()) {
            error_sys(argv[0], "couldn't unlock request queue");
            close(req_fifo);
            unlink(req_fifo_path);
            return errno;
        }
    }*/

    // DEV
    printf("exited in %ds\n", (int)(time(NULL)-initial));

    // Transfered to atexit()
    /*if (free_sync()) {
        error_sys(argv[0], "couldn't free synchronization system");
        close(req_fifo);
        unlink(req_fifo_path);
        return errno;
    }*/


    if (close(req_fifo)) {
        error_sys(argv[0], "couldn't close public request FIFO");
        unlink(req_fifo_path);
        return errno;
    }

    if (unlink(req_fifo_path)) {
        error_sys(argv[0], "couldn't unlink public request FIFO");
        return errno;
    }

    pthread_exit(0);
}

void *th_operation(void *arg) {
    if (arg == NULL) {
        return NULL;
    }

    request_t *request = (request_t*)arg;

    request_t reply;
    if (bath_open) fill_reply(&reply, request->id, request->pid, request->tid, request->dur, order++);
    else fill_reply_error(&reply, request->id, request->pid, request->tid);
    free(request);

    if(write_log(&reply, (bath_open ? "ENTER" : "2LATE"))) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't write log");
    }

    sem_t *sem_reply;

    if ((sem_reply = sem_open_reply(reply.pid, reply.tid)) == NULL) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        if (errno == ENOENT) { //  server couldn't write reply due to client closing the private FIFO
            if(write_log(&reply, "GAVUP")) {
                error_sys(program, "couldn't write log");
            }
        } else {
            error_sys(program, "couldn't open private reply semaphore");
            sem_post_reply(sem_reply);
            return NULL;
        }
        return NULL;
    }

    struct sigaction pipe_action;
    pipe_action.sa_handler = SIG_IGN;
    sigemptyset(&pipe_action.sa_mask);
    pipe_action.sa_flags = 0;
    if (sigaction(SIGPIPE, &pipe_action, NULL)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't install SIGPIPE signal handler");
        sem_post_reply(sem_reply);
        return NULL;
    }

    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", reply.pid, reply.tid);
    int reply_fifo;

    if ((reply_fifo = open(reply_fifo_path, O_WRONLY | O_NONBLOCK)) == -1) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        if (errno == ENXIO || errno == ENOENT) { //  server couldn't write reply due to client closing the private FIFO
            if(write_log(&reply, "GAVUP")) {
                error_sys(program, "couldn't write log");
            }
        } else {
            error_sys(program, "couldn't open private reply FIFO");
            sem_post_reply(sem_reply);
            return NULL;
        }
    }

    if (reply_fifo != -1) {
        if (write_reply(reply_fifo, &reply)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            if (errno == EPIPE) { //  server couldn't write reply due to client closing the private FIFO
                if(write_log(&reply, "GAVUP")) {
                    char program[BUFFER_SIZE];
                    sprintf(program, "reply %d", reply.id);
                    error_sys(program, "couldn't write log");
                }
            } else {
                error_sys(program, "couldn't write to private reply FIFO");
                sem_post_reply(sem_reply);
                close(reply_fifo);
                return NULL;
            }
        }

        if (sem_post_reply(sem_reply)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't notify private reply semaphore");
        }

        if (close(reply_fifo)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't close private reply FIFO");
        }
    }

    if (reply.dur > 0) {
        if (usleep(reply.dur)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't process request duration");
            return NULL;
        }

        if (write_log(&reply, "TIMUP")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    }

    return NULL;
}

int request_queue_not_empty() {
    int locked;
    if (sem_getvalue_send_request(&locked))
        return -1;

    return (locked == 0) ? 1 : 0;
}
