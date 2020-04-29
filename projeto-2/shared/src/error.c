/* MAIN HEADER */
#include "error.h"

/* INCLUDE HEADERS */
#include "constants.h"

/* C LIBRARY HEADERS */
#include <errno.h>
#include <stdio.h>

int error_sys(char *program, char *error_msg) {
    char error[BUFFER_SIZE];
    sprintf(error, "%s: %s", program, error_msg);
    perror(error);
    return errno;
}

int error_sys_ignore_alarm(char *program, char *error_msg, int alarm_status) {
    if (errno == EINTR && alarm_status == ALARM_TRIGGERED) {
        return 0;
    }
    char error[BUFFER_SIZE];
    sprintf(error, "%s: %s", program, error_msg);
    perror(error);
    return errno;
}
