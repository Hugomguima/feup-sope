#include "sig_handler.h"

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
  pid_t pgid = getpgrp();
  (void) signo;

  //FILE* saved_stdout,saved_stdout;

  //Gravar o atual estado do STDOUT_FILENO:
  //saved_stdout = dup(1);
  //dup2(my_temporary_stdout_fd, STDOUT_FILENO);

  //Restauração do atigo estado do STDOUT_FILENO;
  //dup2(saved_stdout, 1);
  //close(saved_stdout);

  killpg(pgid,SIGCONT);
//Deve enviar SIGSTOP para todos os processos filho
//kill com 0 no 1º parâmetro envia o sinal para todos os processos que possuem o mesmo PGID que o processo;
  write(STDOUT_FILENO,"\nAre you sure you want to exit the proggram?\n",45);
  write(STDOUT_FILENO,"Press 'y' to confirm, anything else otherwise\n",46);

  read(STDIN_FILENO,&ch,1);
  if(ch == 'y'){
    killpg(pgid,SIGTERM); // Termina todos os processos parados anteriormente
  }
  else{
    //killpg(pgid,SIGCONT); // Continua todos os processos parados anteriormente
  }
}
