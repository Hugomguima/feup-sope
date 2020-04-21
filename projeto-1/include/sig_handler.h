#ifndef SIG_HANDLER_H_INCLUDED
#define SIG_HANDLER_H_INCLUDED

/**
 * @brief           Sets the global variable globalProcess to pgid and indicates there are subprocesses
 * param pgid       pgid that will hange globalProcess
 */
void setGlobalProcess(int pgid);

/**
 * @brief           Sets the global variable globalProcess to 0 and indicates there are no subprocesses
 */
void resetGlobalProcess(void);

/**
 * @brief           Handler for SIGINT that stops all processes and allows the user to terminate or continue the program
 * param signo      int value for the signal received in the handler (SIGINT)
 */
void sigint_handler(int signo);

/**
 * @brief           Handler for SIGTERM and SIGCONT, which allows writing in log.txt
 * param signo      int value for the signal received in the handler (SIGTERM or SIGSTOP)
 */
void siglog_handler(int signo);

#endif /* end of include guard: SIG_HANDLER_H_INCLUDED */
