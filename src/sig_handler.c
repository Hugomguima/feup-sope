#include "sig_handler.h"

/* C LIBRARY HEADERS */
#include <stdlib.h>
#include <stdio.h>

/* SYSTEM CALLS HEADERS */
#include <unistd.h>

/* SIGNAL HEADERS */
#include <signal.h>

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

//Handler para quando o processo pai recebe SIGINT
void sigint_handler(int signo){
  char ch[256];
  (void) signo;


  if(check_process) killpg(globalProcess,SIGSTOP);  //Envia SIGSTOP para todos os subprocessos, se existirem

  write(STDOUT_FILENO,"\nAre you sure you want to exit the program?\n",44);
  write(STDOUT_FILENO,"Press 'y' to confirm, anything else otherwise\n",46);

  int n = read(STDIN_FILENO,ch,256);
  if((n == 2 ) && (ch[0] == 'y')){
      if(check_process) killpg(globalProcess,SIGTERM); // Termina todos os processos parados anteriormente
      raise(SIGTERM);
  }
  else{
      if(check_process) killpg(globalProcess,SIGCONT); // Continua todos os processos parados anteriormente
  }
}

void sigcont_handler(int signo){
    (void) signo;
}

void sigtermn_handler(int signo){
    (void) signo;
}
