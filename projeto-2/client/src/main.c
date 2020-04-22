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

void *thread_toilet(void *path) {
    request_t req;
    req.i = 2;

    req.pid = getpid();

    int req_fifo = open(path, O_WRONLY);
    write(req_fifo, &req, sizeof(request_t));
    close(req_fifo);

    char ans_fifo_path[1024];
    sprintf(ans_fifo_path, "/tmp/%d", req.pid);
    mkfifo(ans_fifo_path, 0660);
    int ans_fifo = open(ans_fifo_path, O_RDONLY);

    double id;
    read(ans_fifo, &id, sizeof(double));
    close(ans_fifo);
    unlink(ans_fifo_path);

    printf("%lf\n", id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        char error[BUFFER_SIZE];
        char *s = strerror(EINVAL);
        sprintf(error, "Error %d: %s\n"
                       "Program usage: Un <-t nsecs> fifoname\n",
                EINVAL, s);
        write(STDERR_FILENO, error, strlen(error));
        return EINVAL;
    }
    
    char *path_name = argv[2];

    pthread_t tid;
    for(int i = 0; i < atoi(argv[1]); i++) {
        pthread_create(&tid, NULL, thread_toilet, path_name);
        sleep(1);
    }
    return 0;
}
