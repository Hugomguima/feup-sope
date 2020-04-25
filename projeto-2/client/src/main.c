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

double ID_ORDER = 0;

typedef struct {
    double id;
    pid_t pid;
    pthread_t tid;
    int dur;
    int pl;
} request_t;

#define BUFFER_SIZE 256

char *req_fifo_path;

//pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // mutex

void *th_request(void *arg){
    request_t *req = arg;

    //pthread_mutex_lock(&mut);
    int req_fifo = open(req_fifo_path, O_WRONLY);
    write(req_fifo, req, sizeof(request_t));
    close(req_fifo);

    char ans_fifo_path[1024];
    sprintf(ans_fifo_path, "/tmp/%d.tid", req->pid);
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

    if (argc != 3) { 
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\nWithout flags [-l nplaces] [-n nthreads] in the first submission.\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }
   
    long int finish_time = time(NULL) + atoi(argv[1]);
    req_fifo_path = argv[2];

    while(time(NULL) < finish_time) {
        request_t *req = malloc(sizeof(request_t));

        pthread_t tid;

        req->id = ID_ORDER;
        req->pid = getpid();
        req->tid = 0;
        req->dur = 10;
        req->pl = -1;
        
        
        pthread_create(&tid, NULL, th_request, req);
        free(req);

        ID_ORDER++;
    }
    
    return 0;
}