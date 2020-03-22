/* MAIND HEARDER */
#include "log.h"

/* INCLUDE HEADERS */
#include <utils.h>

/* SYSTEM CALLS HEADERS */
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct timeval init_time;
int file_log;

int init_log() {
    if(getenv("LOG_FILENAME") != NULL) { // Write log to LOG_FILENAME
        file_log = open("LOG_FILENAME" , O_WRONLY | O_CREAT, DEFAULT_MODE);
    }
    else { // Write log to a pre defined file
        struct stat st;
        if (stat("./log", &st) == -1) {
            mkdir("./log", 0700);
        }
        file_log = open("./log/log.txt", O_WRONLY | O_CREAT, DEFAULT_MODE);
    }

    if (file_log == -1) { // Test if file is open
        printf("%s\n", strerror(errno));
        return 1;
    }
    gettimeofday(&init_time, 0);
    // Write the first line of log
    char *str = "instant – pid – action – info \n";
    write(file_log, str, strlen(str));

    return 0;
}

int write_log(char *log_string) {
    struct timeval current_time;
    gettimeofday(&current_time, 0);

    // Write time
    char tmp[12];
    long double elapsed_time = (current_time.tv_usec - init_time.tv_usec) / 1000.0 + (current_time.tv_sec - init_time.tv_sec) * 1000.0;
    sprintf(tmp,"%11Lf", elapsed_time);
    write(file_log, tmp, sizeof(tmp));

    // Write action and info
    write(file_log, log_string, strlen(log_string));
    return 0;
}

int close_log() {
    if (close(file_log) != 0) {
        printf("%s", strerror(errno));
        return 1;
    }
    return 0;
}