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

double ID_ORDER = 0;

typedef struct {
    double id;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} request_t;

#define BUFFER_SIZE 256

char req_fifo_path[256];

void *th_request(void *arg){
    request_t *req = arg;
    req->id = ID_ORDER;
    req->pid = getpid();
    req->tid = pthread_self();
    req->dur = 10;
    req->pl = -1;

    //pthread_mutex_lock(&mut);
    int req_fifo = open(req_fifo_path, O_WRONLY);
    write(req_fifo, req, sizeof(request_t));
    close(req_fifo);

    char ans_fifo_path[1024];
    sprintf(ans_fifo_path, "/tmp/%d.%ld", req->pid, req->tid);
    mkfifo(ans_fifo_path, 0660);
    int ans_fifo = open(ans_fifo_path, O_RDONLY);

    double ans;
    read(ans_fifo, &ans, sizeof(double));
    close(ans_fifo);
    unlink(ans_fifo_path);
    printf("%lf\n", ans);
    //pthread_mutex_unlock(&mut);
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

    // error | | fifoname | secs
    int flags;
    parse_info_t info;
    init_parse_info(&info);
    flags = parse_cmd(argc - 1, &argv[1], &info);

    if (flags & FLAG_ERR) {
        free_parse_info(&info);
        return -1;
    }

    long int finish_time = time(NULL) + info.exec_secs;
    sprintf(req_fifo_path, "/tmp/%s", info.path);

    while(time(NULL) < finish_time) {
        request_t *req = malloc(sizeof(request_t));

        pthread_t tid;

        pthread_create(&tid, NULL, th_request, req);
        free(req);

        ID_ORDER++;
    }

    return 0;
}
