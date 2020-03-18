/* MAIND HEARDER */
#include "log.h"

/* INCLUDE HEADERS */
#include <utils.h>

/* SYSTEM CALLS HEADERS */
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

/* C LIBRARY HEADERS */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

struct timeval initTime;
int fileLog;

int init_log() {
    if(getenv("LOG_FILENAME") != NULL) { // Write log to LOG_FILENAME
        fileLog = open("LOG_FILENAME" , O_WRONLY | O_CREAT, 0644);
    }
    else { // Write log to a pre defined file
        fileLog = open("log.txt", O_WRONLY | O_CREAT, 0644);
    }

    if (fileLog == -1) { // Test if file is open
        printf("%s\n", strerror(errno));
        return 1;
    }
    gettimeofday(&initTime, NULL);

    // Write the first line of log
    char *str = "instant – pid – action – info \n";
    write(fileLog, str, strlen(str));

    return 0;
}

int write_log(char *logString) {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    long int elapsedTime = (currentTime.tv_usec - initTime.tv_usec) / 1000;
    printf("Clock: %8ld\n", elapsedTime);
    write(fileLog, logString, strlen(logString));
    return 0;
}

int close_log() {
    if (close(fileLog) != 0) {
        printf("%s", strerror(errno));
        return 1;
    }
    return 0;
}
