#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <string.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 256

typedef struct {
    double id;
    pid_t pid;
    /*
    pthread_t tid;
    int dur;
    int pl;
    */
} request_t;

void *th_operation(void *arg){
    request_t *req = arg;
    printf("%lf\n", req->id);

    char buf[1024];
    sprintf(buf, "/tmp/%d.tid", req->pid);
    int ans_fifo = open(buf, O_WRONLY);
    write(ans_fifo, &req->id, sizeof(double));
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

    char *req_fifo_path = argv[2];

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