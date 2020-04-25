/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "parse.h"
#include "protocol.h"
#include "sync.h"
#include "utils.h"

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
#include <string.h>

/* Miscellaneous */
#include <pthread.h>

int ID_ORDER = 0;
#define MAX_THREADS     100

#define BUFFER_SIZE 256

char SEM_CLIENT[] = "/tmp/client_sem";

void *th_request(void *arg) {
    // É necessário fazer um while loop que seja capaz de verificar se há informação a ser passada (Busy waiting)
    if (arg == NULL) return NULL;

    request_t request;

    int req_fifo = *((int*)arg);

    fill_request(&request, ID_ORDER++, getpid(), pthread_self(), 10/*5 + random() % 50*/);

    char reply_fifo_path[BUFFER_SIZE];
    sprintf(reply_fifo_path, "/tmp/%d.%ld", request.pid, request.tid);


    int reply_fifo = open(reply_fifo_path, O_CREAT | O_RDONLY);
    mkfifo(reply_fifo_path, 0660);

    sem_t *sem_reply;

    if (sem_open_reply(sem_reply, request.pid, request.tid)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        return NULL;
    }

    if (sem_wait_send_request()) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        return NULL;
    }
    if (write_request(req_fifo, &request)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        return NULL;
    }
    if (sem_post_receive_request()) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        return NULL;
    }

    if (sem_wait_reply(sem_reply)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        return NULL;
    }

    request_t reply;
    if (read_reply(reply_fifo, &reply)) {
        close(reply_fifo);
        unlink(reply_fifo_path);
        sem_free_reply(sem_reply, request.pid, request.tid);
        return NULL;
    }

    if (sem_free_reply(sem_reply, request.pid, request.tid)) {
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

    printf("Reply ID: %d\nReply PID: %d\nReply TID: %ld\nReply Dur: %d\nReply PL: %d", reply.id, reply.pid, reply.tid, reply.dur, reply.pl);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Qn <-t nsecs> fifoname", EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }


    pthread_t threads[MAX_THREADS];

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

    long int finish_time = time(NULL) + info.exec_secs;
    sprintf(req_fifo_path, "/tmp/%s", info.path);

    int req_fifo;
    if ((req_fifo = open(req_fifo_path, O_WRONLY)) == -1) {
        perror("fifo error");
        return errno;
    }

    free_parse_info(&info);

    int thread_counter = 0;

    while(time(NULL) < finish_time) {
        usleep(10000);
        printf("%d %d\n", thread_counter, MAX_THREADS);
        if(thread_counter >= MAX_THREADS){
            printf("Reached %d threads (Max Ammount)\n", thread_counter);
            break;
        }
        else {
            pthread_create(&threads[thread_counter], NULL, th_request, &req_fifo);
            thread_counter++;
        }
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
