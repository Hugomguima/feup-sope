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
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TEMPORARY
#include <time.h>

// Check if still sending requests
int requesting = 1;

// Request ID Order
int id_order = 0;

// Alarm status & Handler
int alarm_status = ALARM_CHILL;

void end_requesting(int signo);

// Main Thread ID
pthread_t main_thread_tid;

// Public Request FIFO descriptor
int req_fifo;

// Cleanup
void cleanup(void);

// Request sender
void* request_sender(void *arg);

/* -------------------------------------------------------------------------
                            MAIN THREAD
/-------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {

    if (argc < 4) {
        char error[BUFFER_SIZE];
        sprintf(error, "%s: error %d: %s\n"
                       "Program usage: Qn <-t nsecs> fifoname\n", argv[0], EINVAL, strerror(EINVAL));
        write(STDERR_FILENO, error, strlen(error));
        return -1;
    }

    /* -------------------------------------------------------------------------
                                PARSING
    /-------------------------------------------------------------------------*/
    // error | | fifoname | secs
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        return -1;
    }

    // Public Request FIFO
    char req_fifo_path[BUFFER_SIZE];
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
                                OPENING PUBLIC REQUEST FIFO
    /-------------------------------------------------------------------------*/
    req_fifo = open(req_fifo_path, O_WRONLY | O_NONBLOCK);

    if (req_fifo == -1) {
        if (errno == ENXIO || errno == ENOENT) {
            char error[BUFFER_SIZE];
            sprintf(error, "no public request FIFO of name %s", req_fifo_path);
            error_sys(argv[0], error);
        } else {
            error_sys(argv[0], "couldn't open public request FIFO");
        }
        return -1;
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
                                INSTALL ALARM
    /-------------------------------------------------------------------------*/
    struct sigaction alarm_action;
    alarm_action.sa_handler = end_requesting;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        error_sys(argv[0], "couldn't install alarm");
        return -1;
    }

    alarm(exec_secs);
    time_t initial = time(NULL);

    /* -------------------------------------------------------------------------
                            PREPARE REQUEST ERROR HANDLER
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
                                REQUEST SENDING
    /-------------------------------------------------------------------------*/
    main_thread_tid = pthread_self();

    pthread_t tid;

    // main loop
    while (requesting) {

        // create requesting thread
        int *arg = (int*)malloc(sizeof(int));
        *arg = id_order; // request order id

        if (pthread_create(&tid, NULL, request_sender, arg)) {
            error_sys(argv[0], "couldn't create thread to send request");
            free(arg);
            usleep(1000); // sleep and try again
            continue;
        }

        if (pthread_detach(tid)) {
            usleep(1000); // sleep and try again
            if (pthread_detach(tid)) {
                char error[BUFFER_SIZE];
                sprintf(error, "couldn't detach processing thread %ld", tid);
                error_sys(argv[0], error);
            }
        }

        id_order++;
        usleep(1000);
    }

    // DEV
    printf("Closed in %ds\n", (int)(time(NULL) - initial));

    pthread_exit(NULL);
}

/* -------------------------------------------------------------------------
                ACTION TO EXECUTE UPON ALARM SIGNAL
/-------------------------------------------------------------------------*/
void end_requesting(int signo) {
    requesting = 0;
    alarm_status = ALARM_TRIGGERED;
}

/* -------------------------------------------------------------------------
                            CLEANUP FUNCTION
/-------------------------------------------------------------------------*/
void cleanup(void) {
    if (close(req_fifo)) {
        error_sys("cleanup", "couldn't close public request FIFO");
    }
}

/* -------------------------------------------------------------------------
                            REQUEST SENDER
/-------------------------------------------------------------------------*/
void* request_sender(void *arg) {
    if (arg == NULL) {
        return NULL;
    }

    int order = *((int*)arg);

    /* -------------------------------------------------------------------------
                                CREATE REQUEST
    /-------------------------------------------------------------------------*/
    request_t request;
    fill_request(&request, order, getpid(), pthread_self());

    free(arg);

    /* -------------------------------------------------------------------------
                                CREATE REPLY FIFO
    /-------------------------------------------------------------------------*/
    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", request.pid, request.tid);

    if (mkfifo(reply_fifo_path, 0660)) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't create private reply FIFO");
        return NULL;
    }

    /* -------------------------------------------------------------------------
                                SEND REQUEST
    /-------------------------------------------------------------------------*/
    while (1) {
        int ret;
        ret = write(req_fifo, &request, sizeof(request_t));

        // on error
        if (ret == -1) {
            if (errno == EPIPE) { // Server closed request FIFO
                if (alarm_status == ALARM_CHILL) { // stop further requesting
                    alarm(0);
                    pthread_kill(main_thread_tid, SIGALRM);
                }
                if (write_log(&request, "CLOSD")) {
                    char program[BUFFER_SIZE];
                    sprintf(program, "request %d", request.id);
                    error_sys(program, "couldn't write log");
                }
                unlink(reply_fifo_path);
                return NULL;
            } else if (errno == EAGAIN) { // Request FIFO is full, try again after some time
                usleep(FIFO_WAIT_TIME);
                continue;
            } else { // Couldn't send request
                char program[BUFFER_SIZE];
                sprintf(program, "request %d", request.id);
                error_sys(program, "couldn't write to public request FIFO");
                unlink(reply_fifo_path);
                return NULL;
            }
        }
        // on success
        if (write_log(&request, "IWANT")) {
            char program[BUFFER_SIZE];
            sprintf(program, "request %d", request.id);
            error_sys(program, "couldn't write log");
        }
        break;
    }

    /* -------------------------------------------------------------------------
                        OPENING PRIVATE REPLY FIFO
    /-------------------------------------------------------------------------*/
    int reply_fifo;
    if ((reply_fifo = open(reply_fifo_path, O_RDONLY)) == -1) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        // wasn't alarm caused
        if (error_sys_ignore_alarm(program, "couldn't open private reply FIFO", alarm_status)) {
            unlink(reply_fifo_path);
            return NULL;
        }
        // it was alarm, try again
        if ((reply_fifo = open(reply_fifo_path, O_RDONLY)) == -1) {
            error_sys(program, "couldn't open private reply FIFO");
            unlink(reply_fifo_path);
            return NULL;
        }
    }

    /* -------------------------------------------------------------------------
                                REPLY READING
    /-------------------------------------------------------------------------*/
    request_t reply;

    int ret;
    ret = read(reply_fifo, &reply, sizeof(request_t));

    if (ret == -1) { // error
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't read from private reply FIFO");
        unlink(reply_fifo_path);
        close(reply_fifo);
        return NULL;
    } else if (ret == 0) { // EOF
        if (write_log(&request, "FAILD")) {
            char program[BUFFER_SIZE];
            sprintf(program, "request %d", request.id);
            error_sys(program, "couldn't write log");
        }
        unlink(reply_fifo_path);
        close(reply_fifo);
        return NULL;
    }
    // on sucess

    /* -------------------------------------------------------------------------
                                PROCESS REPLY
    /-------------------------------------------------------------------------*/
    if (reply.pl != -1) {
        if (write_log(&reply, "IAMIN")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    } else {
        if (alarm_status == ALARM_CHILL) { // stop further requesting
            alarm(0);
            pthread_kill(main_thread_tid, SIGALRM);
        }
        if (write_log(&reply, "CLOSD")) {
            char program[BUFFER_SIZE];
            sprintf(program, "reply %d", reply.id);
            error_sys(program, "couldn't write log");
        }
    }

    if (unlink(reply_fifo_path)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't unlink private reply FIFO");
    }

    if (close(reply_fifo)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't close private reply FIFO");
    }

    return NULL;
}
