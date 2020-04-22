#include "parse.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 255

typedef struct {
    double i; 
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} request_t;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // mutex

void *th_operation(void *arg) {
    request_t *req = arg;
    double id = req->i; 

    char buf[1024];
    sprintf(buf, "/tmp/%d", req->pid);
    int ans_fifo = open(buf, O_WRONLY);
    write(ans_fifo, &id, sizeof(double));
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 3) { 
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\nWithout flags [-l nplaces] [-n nthreads] in the first submission.\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }

    char *path_name = argv[2];

    mkfifo(path_name, 0660);
    int req_fifo = open(path_name, O_RDONLY);

    request_t *req = malloc(sizeof(request_t));
    pthread_t tid;
    int r;
    while((r = read(req_fifo, req, sizeof(request_t))) != -1) {
        pthread_create(&tid, NULL, th_operation, req);
    }
    free(req);
    close(req_fifo);
    return 0;
}

