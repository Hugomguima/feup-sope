/* MAIN HEADER */
#include "sig_handler.h"

/* INCLUDE HEADERS */
#include "log.h"

/* SYSTEM CALLS HEADERS */
#include <signal.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <stdlib.h>
#include <stdio.h>

int globalProcess = 0;
int check_process = 0;

void setGlobalProcess(int pgid){
    globalProcess = pgid;
    check_process = 1;
}

void resetGlobalProcess(void){
    globalProcess = 0;
    check_process = 0;
}

// Handler for SIGINT
void sigint_handler(int signo){
    (void)signo;

    write_log("RECV_SIGNAL", "SIGINT\n");

    char ch[256];

    if(check_process) {
        write_log_sign("SEND_SIGNAL", "SIGSTOP", globalProcess);
        // Send sigstops for every subprocess
        killpg(globalProcess, SIGSTOP);  
    }

    write(STDOUT_FILENO,"\nAre you sure you want to exit the program?\n",44);
    write(STDOUT_FILENO,"Press 'y' to confirm, anything else otherwise\n",46);

    int n = read(STDIN_FILENO,ch,256);
    if((n == 2 ) && (ch[0] == 'y')){
        if(check_process) {
            write_log_sign("SEND_SIGNAL", "SIGTERM", globalProcess);
            // Shutdown previos process
            killpg(globalProcess,SIGTERM); 
        }
        raise(SIGTERM);
    }
    else{
        if(check_process){
            write_log_sign("SEND_SIGNAL", "SIGSTOP", globalProcess);
            // Continue all previous stopped processes
            killpg(globalProcess,SIGCONT); 
        }
    }
}

void siglog_handler(int signo){
    if(signo == SIGTERM){
        write_log_int("RECV_SIGNAL", "SIGTERM\n");

        struct sigaction newHandler;

        newHandler.sa_handler = SIG_DFL;
        sigemptyset(&newHandler.sa_mask);
        newHandler.sa_flags = 0;
        sigaction(SIGTERM,&newHandler,NULL);
        raise(SIGTERM);
    }
    else if(signo == SIGCONT){
        write_log_int("RECV_SIGNAL", "SIGCONT\n");
    }

}