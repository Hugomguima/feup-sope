include "sig_handler.h"

/* C LIBRARY HEADERS */
#include <stdlib.h>
#include <stdio.h>

/* SYSTEM CALLS HEADERS */
#include <unistd.h>

/* SIGNAL HEADERS */
#include <signal.h>

//Handler para quando o processo pai recebe SIGINT
void sigint_handler(int signo){
  char ch;
//Deve enviar SIGSTOP para todos os proessos filho
//kill com 0 no 1º parâmetro envia o sinal para todos os processos que possuem o mesmo PGID que o processo;
  kill(0,SIGSTOP);
  write(STDOUT_FILENO,"Are you sure you want to exit the proggram?\n",44);
  write(STDOUT_FILENO,"Press 'y' to confirm, anything else otherwise\n",46);
  read(STDIN_FILENOT_FILENO,&ch,1);
  if(ch == 'y'){
    kill(0,SIGTERM); // Termina todos os processos parados anteriormente
  else{
    kill(0,SIGCONT); // Continua todos os processos parados anteriormente
  }
}

void handler_install(){
  struct sigaction action;
  action.sa_handler = sigint_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGINT,&action,NULL) < 0){
   fprintf(stderr,"Unable to install SIGINT handler\n");
   exit(1);
 }
}
