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

// Check if still accepting requests
int bath_open = 1;

// Alarm status & Handler
int alarm_status = ALARM_CHILL;

void close_bathroom(int signo);

// Cleanup
void cleanup(void);

// Order of entrance
int order = 1;

// Public Request FIFO
char req_fifo_path[BUFFER_SIZE];

// Request processor
void* request_processor(void *arg);

/* -------------------------------------------------------------------------
                            MAIN THREAD
/-------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {

    if (argc < 4) {
        char error[BUFFER_SIZE];
        sprintf(error, "%s: error %d: %s\n"
                       "Program usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n", argv[0], EINVAL, strerror(EINVAL));
        write(STDERR_FILENO, error, strlen(error));
        return -1;
    }

    /* -------------------------------------------------------------------------
                                PARSING
    /-------------------------------------------------------------------------*/
    // error | | | | fifoname | threads | places | secs
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        return -1;
    }

    // Public Request FIFO
    if (sprintf(req_fifo_path, "/tmp/%s", info.path) < 0) {
        char error[BUFFER_SIZE];
        sprintf(error, "error creating public request FIFO path\n");
        write(STDERR_FILENO, error, strlen(error));
        return -1;
    }

    // Execution duration
    int exec_secs = info.exec_secs;

    free_parse_info(&info);

    /* -------------------------------------------------------------------------
                                INSTALL ALARM
    /-------------------------------------------------------------------------*/
    struct sigaction alarm_action;
    alarm_action.sa_handler = close_bathroom;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        error_sys(argv[0], "couldn't install alarm");
        return -1;
    }

    alarm(exec_secs);
    time_t initial = time(NULL);

    /* -------------------------------------------------------------------------
                                PREPARE REPLY ERROR HANDLER
    /-------------------------------------------------------------------------*/
    struct sigaction pipe_action;
    pipe_action.sa_handler = SIG_IGN;
    sigemptyset(&pipe_action.sa_mask);
    pipe_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        error_sys(argv[0], "couldn't install reply error handler");
        return -1;
    }

    /* -------------------------------------------------------------------------
                                OPENING PUBLIC REQUEST FIFO
    /-------------------------------------------------------------------------*/

    if (mkfifo(req_fifo_path, 0660)) {
        error_sys(argv[0], "couldn't create public request FIFO");
        return -1;
    }

    int req_fifo;
    if ((req_fifo = open(req_fifo_path, O_RDONLY)) == -1) {
        if (error_sys_ignore_alarm(argv[0], "couldn't open public request FIFO", alarm_status)) {
            unlink(req_fifo_path);
            return -1;
        }
    }

    /* -------------------------------------------------------------------------
                                CLEANUP
    /-------------------------------------------------------------------------*/
    if (atexit(cleanup)) {
        cleanup();
        error_sys(argv[0], "failed to install cleanup");
        return -1;
    }

    /* -------------------------------------------------------------------------
                                REQUEST READING
    /-------------------------------------------------------------------------*/
    request_t *request;
    pthread_t tid;

    int empty = 1;

    int ret;
    // main loop
    while (bath_open || !empty) {
        request = (request_t*)malloc(sizeof(request_t));

        ret = read(req_fifo, request, sizeof(request_t));

        if (ret == -1) { // error
            // error wasn't caused by alarm
            if (error_sys_ignore_alarm(argv[0], "couldn't read request queue", alarm_status)) {
                close(req_fifo);
                free(request);
                pthread_exit(NULL);
            }
            // error was caused by alarm, proceed as normal
            free(request);
            continue;
        } else if (ret == 0) { // EOF
            free(request);
            empty = 1;
            continue;
        }

        empty = 0;

        if (write_log(request, "RECVD")) {
            error_sys(argv[0], "couldn't write log");
        }

        // process request
        if (pthread_create(&tid, NULL, request_processor, request)) {
            error_sys(argv[0], "couldn't create thread to process request");
            usleep(1000); // sleep and try again
            if (pthread_create(&tid, NULL, request_processor, request)) {
                error_sys(argv[0], "couldn't create thread to process request");
                free(request);
                close(req_fifo);
                pthread_exit(NULL);
            }
        }

        if (pthread_detach(tid)) {
            usleep(1000); // sleep and try again
            if (pthread_detach(tid)) {
                char error[BUFFER_SIZE];
                sprintf(error, "couldn't detach processing thread %ld", tid);
                error_sys(argv[0], error);
                close(req_fifo);
                pthread_exit(NULL);
            }
        }
    }

    // DEV
    printf("Closed in %ds\n", (int)(time(NULL) - initial));

    if (req_fifo != -1) {
        if (close(req_fifo)) {
            error_sys(argv[0], "couldn't close public request FIFO");
        }
    }

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------
                ACTION TO EXECUTE UPON ALARM SIGNAL
/-------------------------------------------------------------------------*/
void close_bathroom(int signo) {
    bath_open = 0;
    alarm_status = ALARM_TRIGGERED;
    if (req_fifo_path == NULL) return;
    if (unlink(req_fifo_path)) {
        error_sys("closing", "error on unlinking public FIFO");
    }
}

/* -------------------------------------------------------------------------
                            CLEANUP FUNCTION
/-------------------------------------------------------------------------*/
void cleanup(void) {
    if (alarm_status == ALARM_CHILL) {
        if (unlink(req_fifo_path)) {
            error_sys("cleanup", "error on unlinking public FIFO");
        }
    }
}

/* -------------------------------------------------------------------------
                            REQUEST PROCESSOR
/-------------------------------------------------------------------------*/
void* request_processor(void *arg) {
    if (arg == NULL) {
        return NULL;
    }

    request_t *request = (request_t*)arg;
    /* -------------------------------------------------------------------------
                                CREATE REPLY
    /-------------------------------------------------------------------------*/
    request_t reply;

    if (bath_open) {
        fill_reply(&reply, request->id, request->pid, request->tid, request->dur, order++);
        if (write_log(&reply, "ENTER")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    } else {
        fill_reply_error(&reply, request->id, request->pid, request->tid);
        if (write_log(&reply, "2LATE")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    }

    free(request);

    /* -------------------------------------------------------------------------
                                PREPARE REPLY FIFO
    /-------------------------------------------------------------------------*/
    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", reply.pid, reply.tid);

    int reply_fifo;

    reply_fifo = open(reply_fifo_path, O_WRONLY);

    if (reply_fifo == -1) {
        if (errno == ENXIO || errno == ENOENT) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            if (write_log(&reply, "GAVUP")) {
                error_sys(program, "couldn't write log");
            }
        } else {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't open private reply FIFO");
        }
    }

    /* -------------------------------------------------------------------------
                                SEND REPLY
    /-------------------------------------------------------------------------*/
    if (reply_fifo != -1) {
        int ret;

        while (1) {
            ret = write(reply_fifo, &reply, sizeof(request_t));

            // on error
            if (ret == -1) {
                if (errno == EPIPE) {   // Client closed reply FIFO
                    if (write_log(&reply, "GAVUP")) {
                        char program[BUFFER_SIZE];
                        sprintf(program, "reply %d", reply.id);
                        error_sys(program, "couldn't write log");
                    }
                    break;
                /*} else if (errno == EAGAIN) { // Reply FIFO is full, try again after some time
                    usleep(FIFO_WAIT_TIME);
                    continue;*/
                } else { // Couldn't send reply
                    char program[BUFFER_SIZE];
                    sprintf(program, "reply %d", reply.id);
                    error_sys(program, "couldn't write to private reply FIFO");
                    break;
                }
            }
            // on success
            break;
        }

        if (close(reply_fifo)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't close private reply FIFO");
        }
    }

    /* -------------------------------------------------------------------------
                                PROCESS REQUEST USAGE TIME
    /-------------------------------------------------------------------------*/
    if (reply.dur > 0) { // if it was accepted
        if (usleep(reply.dur)) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't process request duration");
        }

        if (write_log(&reply, "TIMUP")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    }

    return NULL;
}
