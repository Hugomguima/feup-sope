#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <sys/time.h>

#define DEFAULT_MODE 0644 /**< @brief Permissions associated with the file */

/**
 * @brief           Init the log
 * @return          File descriptor uppon sucess or 1 otherwise
 */
int init_log();

/**
  * @brief           Set the descriptor of log file in child processes
  * @int             file descriptor
  * @return          0 uppon sucess
  */
int set_log_descriptor(int descriptor);

/**
  * @brief           Set the initial time in child processes
  * @int             initial timeval received through pipe
  * @return          0 uppon sucess
  */
int set_time(struct timeval *it);

/**
  * @brief
  * @return             Return the elapsed time since program started
  */
long double elapsed_time();

/**
 * @brief               Write an action to log
 * @param log_action    Sinal type
 * @param pid           PID of global process
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log_sign(char *log_action, long log_info, int pid);

/**
 * @brief               Write an action to log
 * @param log_action    Timeval struct
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log_timeval(char *log_action, struct timeval log_info);

/**
 * @brief               Write an action to log
 * @param log_action    Description of type of event
 * @param log_info      Additional information about any action
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log(char *log_action, char *log_info);

/**
 * @brief               Write an action to log
 * @param log_action    Description of type of event
 * @param info          int array to write in log
 * @param size          size of array info
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log_array(char *log_action, int *info, int size);

/**
 * @brief               Write an action to log
 * @param log_action    Description of type of event
 * @param info          long int to write in log
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log_int(char *log_action, long log_info);

/**
 * @brief           Close log file
 * @return          0 uppon sucess or 1 otherwhise
 */
int close_log();

#endif // LOG_H_INCLUDED
