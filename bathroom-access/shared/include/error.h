#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

int error_sys(char *program, char *error_msg);

int error_sys_ignore_alarm(char *program, char *error_msg, int alarm_status);

#endif /* end of include guard: ERROR_H_INCLUDED */
