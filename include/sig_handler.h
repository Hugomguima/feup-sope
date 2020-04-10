#ifndef SIG_HANDLER_H_INCLUDED
#define SIG_HANDLER_H_INCLUDED

void setGlobalProcess(int pgid);

void resetGlobalProcess(void);

void sigint_handler(int signo);

void siglog_handler(int signo);

#endif /* end of include guard: SIG_HANDLER_H_INCLUDED */
