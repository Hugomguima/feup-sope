/* MAIN HEADER */

/* INCLUDE HEADERS */
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

#include <time.h>

#define BUFFER_SIZE     256

#define MAX_THREADS     500

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // mutex

int bath_open = 1;

int order = 1;

void sigalarm_handler() {
    bath_open = 0;
}

void *th_operation(void *arg){

    if (arg == NULL) {
        return NULL;
    }

    request_t *request = (request_t*)arg;

    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", request->pid, request->tid);
    int reply_fifo;
    if ((reply_fifo = open(reply_fifo_path, O_WRONLY)) == -1) {
        free(request);
        return NULL;
    }

    sem_t *sem_reply;

    if (sem_open_reply(sem_reply, request->pid, request->tid)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        return NULL;
    }

    request_t reply;
    fill_reply(&reply, request->id, request->pid, request->tid, request->dur, order++);

    free(request);

    if (write_reply(reply_fifo, &reply)) {
        return NULL;
    }

    if (sem_post_reply(sem_reply)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        return NULL;
    }

    if (close(reply_fifo)) {
        return NULL;
    }

    if (unlink(reply_fifo_path)) {
        return NULL;
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
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

    char req_fifo_path[BUFFER_SIZE];
    sprintf(req_fifo_path, "/tmp/%s", info.path);

    free_parse_info(&info);

    if (mkfifo(req_fifo_path, 0660)) {
        perror("make fifo");
        return errno;
    }

    int req_fifo;
    if ((req_fifo = open(req_fifo_path, O_RDONLY | O_NONBLOCK)) == -1) {
        unlink(req_fifo_path);
        perror("open fifo");
        return errno;
    }

    if (init_sync(O_CREAT)) {
        close(req_fifo);
        unlink(req_fifo_path);
        perror("failed to init sync");
        return errno;
    }

    struct sigaction alarm_action;
    alarm_action.sa_handler = sigalarm_handler;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    sigaction(SIGALRM, &alarm_action, NULL);

    pthread_t tid;

    int thread_counter = 0;

    request_t *request;

    time_t initial = time(NULL);

    alarm(info.exec_secs);

    while (bath_open) {
        if (sem_post_send_request()) {
            free_sync();
            close(req_fifo);
            unlink(req_fifo_path);
            return errno;
        }

        if (sem_wait_receive_request()) {
            if (errno != EINTR) {
                free_sync();
                close(req_fifo);
                unlink(req_fifo_path);
                return errno;
            }
        }


        if (bath_open) {
            request = (request_t*)malloc(sizeof(request_t));

            if (read_request(req_fifo, request)) {
                free_sync();
                close(req_fifo);
                unlink(req_fifo_path);
                free(request);
                perror("read request");
                return errno;
            }
            printf("received request");
            if (pthread_create(&tid, NULL, th_operation, request)) {
                free_sync();
                close(req_fifo);
                unlink(req_fifo_path);
                free(request);
                perror("server thread create");
                return errno;
            }

            if (pthread_detach(tid)) {
                free_sync();
                close(req_fifo);
                unlink(req_fifo_path);
                free(request);
                perror("server thread detach");
                return errno;
            }
        }
    }

    printf("exited in %ds\n", (int)(time(NULL)-initial));

    close(req_fifo);
    unlink(req_fifo_path);
    pthread_exit(0);
}
