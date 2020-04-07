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
    printf("Here\n");
    if(getenv("LOG_FILENAME") != NULL) { // Write log to LOG_FILENAME
        file_log = open("LOG_FILENAME" , O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_MODE);
    }
    else { // Write log to a pre defined file
        struct stat st;
        if (stat("./log", &st) == -1) {
            mkdir("./log", 0700);
        }
        file_log = open("./log/log.txt", O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_MODE);
    }

    if (file_log == -1) { // Test if file is open
        printf("%s\n", strerror(errno));
        return 1;
    }

    // Write the first line of log
    char *str = "instant\tpid\taction\tinfo \n";
    write(file_log, str, strlen(str));

    return file_log;
}

int set_log_descriptor(int descriptor) {
    file_log = descriptor;
    return 0;
}

int set_time(struct timeval *it) {
  init_time = *it;
  return 0;
}


long double elapsed_time() {
  struct timeval current_time;
  gettimeofday(&current_time, 0);
  return (current_time.tv_usec - init_time.tv_usec) / 1000.0 + (current_time.tv_sec - init_time.tv_sec) * 1000.0;
}

int write_log(char *log_action, char *log_info) {
    char buffer[256];
    sprintf(buffer,"%10.2Lf\t%15d\t%15s\t%s", elapsed_time(), getppid(), log_action, log_info);

    if(write(file_log, buffer, strlen(buffer)) == -1) {
        return 1;
    }
    return 0;
}

int write_log_timeval(char *log_action, struct timeval log_info) {
    char buffer[256];
    sprintf(buffer,"%10.2Lf\t%15d\t%15s\t%ld%ld\n", elapsed_time(), getppid(), log_action, log_info.tv_sec, log_info.tv_usec);

    if(write(file_log, buffer, strlen(buffer)) == -1) {
        return 1;
    }
    return 0;
}

int write_log_array(char *log_action, int *info, int size) {
    char to_char[256];
    char *log_info = malloc(256);
    for(int i = 0; i < size; i++) {
      sprintf(to_char, "%d", info[i]);
      strncat(log_info, to_char, 256 - 1);
    }

    char buffer[256];
    sprintf(buffer,"%10.2Lf\t%15d\t%15s\t%s\n", elapsed_time(), getppid(), log_action, log_info);
    if(write(file_log, buffer, strlen(buffer)) == -1) {
        free(log_info);
        return 1;
    }
    free(log_info);
    return 0;
}

int write_log_int(char *log_action, long log_info) {
    char buffer[256];
    sprintf(buffer,"%10.2Lf\t%15d\t%15s\t%ld\n", elapsed_time(), getppid(), log_action, log_info);
    if(write(file_log, buffer, strlen(buffer)) == -1) {
        return 1;
    }

    return 0;
}

int close_log() {
    if (close(file_log) != 0) {
        return 1;
    }
    return 0;
}
