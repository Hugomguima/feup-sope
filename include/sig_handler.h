#ifndef SIG_HANDLER_H_INCLUDED
#define SIG_HANDLER_H_INCLUDED

void sigint_handler(int signo);

void setGlobalProcess(int pgid);

void resetGlobalProcess(void);

void sigcont_handler(int signo);


#endif /* end of include guard: SIG_HANDLER_H_INCLUDED */
