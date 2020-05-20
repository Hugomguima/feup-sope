#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

/* MAIN HEADER */

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */
#include <pthread.h>
#include <semaphore.h>

/*----------------------------------------------------------------------------*/
/*                              ALARM                                         */
/*----------------------------------------------------------------------------*/

void *my_alarm(void *arg);

int create_alarm(pthread_t tid, int exec_secs, int signal, pthread_t *ret);

int stop_alarm(pthread_t tid);

/*----------------------------------------------------------------------------*/
/*                              REQUEST SEMAPHORE                             */
/*----------------------------------------------------------------------------*/

int init_sync(int oflags);

int free_sync();

int sem_wait_send_request();

int sem_post_send_request();

int sem_getvalue_send_request(int *val);

int sem_wait_receive_request();

int sem_post_receive_request();

int sem_getvalue_receive_request(int *val);

/*----------------------------------------------------------------------------*/
/*                              REPLY SEMAPHORE                               */
/*----------------------------------------------------------------------------*/

sem_t* sem_open_reply(pid_t pid, pthread_t tid);

int sem_wait_reply(sem_t *sem_reply);

int sem_post_reply(sem_t *sem_reply);

int sem_close_reply(sem_t *sem_reply);

int sem_unlink_reply(const char *sem_name);

int sem_free_reply(sem_t *sem_reply, pid_t pid, pthread_t tid);

#endif /* end of include guard: SYNC_H_INCLUDED */
