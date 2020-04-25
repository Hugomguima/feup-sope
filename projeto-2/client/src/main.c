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
    /*
    pthread_t tid;
    int dur;
    int pl;
    */
} request_t;

#define BUFFER_SIZE 256

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

    while(1) {
        request_t req;

        req.id = ID_ORDER;
        
        req.pid = getpid();
        
        int req_fifo = open(req_fifo_path, O_WRONLY);
        write(req_fifo, &req, sizeof(request_t));
        close(req_fifo);

        char ans_fifo_path[1024];
        sprintf(ans_fifo_path, "/tmp/%d.tid", req.pid);
        mkfifo(ans_fifo_path, 0660);
        int ans_fifo = open(ans_fifo_path, O_RDONLY);

        double ans;
        read(ans_fifo, &ans, sizeof(double));
        close(ans_fifo);
        unlink(ans_fifo_path);
        printf("%lf\n", ans);

        ID_ORDER++;
    }
    
    return 0;
}