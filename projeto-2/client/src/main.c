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
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Miscellaneous */
#include <pthread.h>

int id_order = 0;
int requesting = 1;
int alarm_status = ALARM_CHILL;

sem_t thread_counter;

void *th_request(void *arg);

void free_shared_memory() {
    if (sem_destroy(&thread_counter)) {
        error_sys_ignore_alarm("exit", "couldn't destroy thread semaphore", alarm_status);
    }
}

void end_requesting(int sig) {
    requesting = 0;
    alarm_status = ALARM_TRIGGERED;

    printf("REACHED\n");
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Qn <-t nsecs> fifoname", EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    // error | | fifoname | secs
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        return -1;
    }

    sem_t *client_sem;
    char req_fifo_path[BUFFER_SIZE];

    sprintf(req_fifo_path, "/tmp/%s", info.path);

    int exec_secs = info.exec_secs;

    printf("%d\n", exec_secs);

    free_parse_info(&info);

    if (init_sync(0)) {
        return error_sys(argv[0], "couldn't open synchronization system");
    }

    if (atexit(free_shared_memory)) {
        free_shared_memory();
        return error_sys(argv[0], "couldn't install atexit function");
    }

    sem_init(&thread_counter, SEM_SHARED, 1);

    struct sigaction alarm_action;
    alarm_action.sa_handler = end_requesting;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        return error_sys(argv[0], "couldn't install alarm");
    }

    pthread_t tid;

    // DEV
    #include <time.h>
    time_t initial = time(NULL);
    printf("%d\n", exec_secs);
    alarm(exec_secs);

    while(requesting) {

        if (sem_wait(&thread_counter)) {
            if (error_sys_ignore_alarm(argv[0], "error on waiting empty thread slot", alarm_status)) {
                pthread_exit(0);
            }
            break;
        }

        if (pthread_create(&tid, NULL, th_request, req_fifo_path)) {
            if (error_sys_ignore_alarm(argv[0], "couldn't create request thread", alarm_status)) {
                pthread_exit(0);
            }
            if (pthread_create(&tid, NULL, th_request, req_fifo_path)) {
                error_sys(argv[0], "couldn't create request thread");
                pthread_exit(0);
            }
        }

        if (pthread_detach(tid)) {
            char error[BUFFER_SIZE];
            sprintf(error, "couldn't detach requesting thread %ld", tid);
            if (error_sys_ignore_alarm(argv[0], error, alarm_status)) {
                pthread_exit(0);
            }
            if (pthread_detach(tid)) {
                error_sys(argv[0], error);
                pthread_exit(0);
            }
        }
        if (usleep(MAX_DURATION)) printf("teste\n");
    }

    // DEV
    printf("exited in %ds\n", (int)(time(NULL)-initial));

    pthread_exit(0);
}

void *th_request(void *arg) {
    if (arg == NULL) {
        sem_post(&thread_counter);
        return NULL;
    }

    request_t request;

    char *req_fifo_path = (char*)arg;

    fill_request(&request, id_order++, getpid(), pthread_self());

    if(write_log(&request, "IWANT")) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't write log");
    }

    struct sigaction pipe_action;
    pipe_action.sa_handler = SIG_IGN;
    sigemptyset(&pipe_action.sa_mask);
    pipe_action.sa_flags = 0;
    if (sigaction(SIGPIPE, &pipe_action, NULL)) {
        sem_post(&thread_counter);
        return NULL;
    }

    int req_fifo;
    if ((req_fifo = open(req_fifo_path, O_WRONLY | O_NONBLOCK)) == -1) {
        if (errno == EPIPE) { //  client couldn't open public request FIFO
            if(write_log(&request, "CLOSD")) {
                char program[BUFFER_SIZE];
                sprintf(program, "request %d", request.id);
                error_sys(program, "couldn't write log");
            }
        } else {
            char program[BUFFER_SIZE];
            sprintf(program, "request %d", request.id);
            error_sys(program, "couldn't open public request FIFO");
        }
        sem_post(&thread_counter);
        return NULL;
    }

    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", request.pid, request.tid);

    if (mkfifo(reply_fifo_path, 0660)) {
        close(req_fifo);
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't create private reply FIFO");
        sem_post(&thread_counter);
        return NULL;
    }

    int reply_fifo;
    if ((reply_fifo = open(reply_fifo_path, O_RDONLY | O_NONBLOCK)) == -1) {
        close(req_fifo);
        unlink(reply_fifo_path);
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't open private reply FIFO");
        sem_post(&thread_counter);
        return NULL;
    }

    sem_t *sem_reply;

    if ((sem_reply = sem_open_reply(request.pid, request.tid)) == NULL) {
        close(req_fifo);
        unlink(reply_fifo_path);
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't open private reply semaphore");
        sem_post(&thread_counter);
        return NULL;
    }

    if (sem_wait_send_request()) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't open private reply FIFO");
        close(req_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        sem_post(&thread_counter);
        return NULL;
    }

    if (write_request(req_fifo, &request)) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        if (errno == EPIPE) { //  client couldn't open public request FIFO
            if(write_log(&request, "CLOSD")) {
                error_sys(program, "couldn't write log");
            }
        } else {
            error_sys(program, "couldn't write to public request FIFO");
        }
        close(req_fifo);
        unlink(reply_fifo_path);
        sem_post_receive_request();
        sem_free_reply(sem_reply, request.pid, request.tid);
        sem_post(&thread_counter);
        return NULL;
    }
    if (sem_post_receive_request()) {
        close(req_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't write to public request FIFO");
        sem_post(&thread_counter);
        return NULL;
    }

    int private_alarm_status = ALARM_CHILL;
    void private_alarm(int sig) {
        private_alarm_status = ALARM_TRIGGERED;
    }

    struct sigaction alarm_action;
    alarm_action.sa_handler = private_alarm;
    sigemptyset(&alarm_action.sa_mask);
    alarm_action.sa_flags = 0;
    if (sigaction(SIGALRM, &alarm_action, NULL)) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        error_sys(program, "couldn't install alarm");
    }

    alarm(REPLY_TOLERANCE);

    if (sem_wait_reply(sem_reply)) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        if (error_sys_ignore_alarm(program, "error on waiting for private reply FIFO", private_alarm_status) == 0) {
            if (write_log(&request, "FAILD")) {
                error_sys(program, "couldn't write log");
            }
        }
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        sem_post(&thread_counter);
        return NULL;
    }

    alarm(0);

    request_t reply;
    if (read_reply(reply_fifo, &reply)) {
        char program[BUFFER_SIZE];
        sprintf(program, "request %d", request.id);
        if (errno == EPIPE) { //  client couldn't open public request FIFO
            if(write_log(&request, "CLOSD")) {
                error_sys(program, "couldn't write log");
            }
        } else {
            error_sys(program, "couldn't read private reply FIFO");
        }
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        sem_post(&thread_counter);
        return NULL;
    }

    if (write_log(&reply, "IAMIN")) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't write log");
    }

    if (sem_free_reply(sem_reply, request.pid, request.tid)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't free private reply semaphore");
    }

    if (close(reply_fifo)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't close private reply FIFO");
    }

    if (unlink(reply_fifo_path)) {
        char program[BUFFER_SIZE];
        sprintf(program, "reply %d", reply.id);
        error_sys(program, "couldn't unlink private reply FIFO");
    }
    sem_post(&thread_counter);
    return NULL;
}
