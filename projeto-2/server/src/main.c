/* MAIN HEADER */

/* INCLUDE HEADERS */
#include "parse.h"
#include "utils.h"

/* SYSTEM CALLS HEADERS */
#include <fcntl.h>
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

#define BUFFER_SIZE 256

typedef struct {
    double id;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} request_t;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // mutex

void *th_operation(void *arg){
    request_t *req = arg;
    printf("%lf\n", req->id);

    char buf[1024];
    sprintf(buf, "/tmp/%d.%ld", req->pid, req->tid);
    int ans_fifo = open(buf, O_WRONLY);
    write(ans_fifo, &req, sizeof(request_t));
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

    char req_fifo_path[256];
    sprintf(req_fifo_path, "/tmp/%s", info.path);

    mkfifo(req_fifo_path, 0660);
    int req_fifo = open(req_fifo_path, O_RDONLY);

    request_t *req = malloc(sizeof(request_t));
    pthread_t tid;
    int r;
    while((r = read(req_fifo, req, sizeof(request_t))) != -1){
        if(r != sizeof(request_t)) continue;
        pthread_create(&tid, NULL, th_operation, req);
    }

    close(req_fifo);
    unlink(req_fifo_path);
    return 0;
}