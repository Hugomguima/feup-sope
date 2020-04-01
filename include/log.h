#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define DEFAULT_MODE 0644 /**< @brief Permissions associated with the file */

/**
 * @brief           Init the log
 * @return          File descriptor uppon sucess or 1 otherwise
 */
int init_log();

/**
  */
int set_log_descriptor(int descriptor);

/**
 * @brief               Write a line to log
 * @param log_action    Description of type of event
 * @param log_info      Additional information about any action
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log(char *log_action, char *log_info);

int write_log_array(char *log_action, int *info, int size);

int write_log_int(char *log_action, long info);

/**
 * @brief           Close log file
 * @return          0 uppon sucess or 1 otherwhise
 */
int close_log();

#endif // LOG_H_INCLUDED
