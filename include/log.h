#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define DEFAULT_MODE 0644 /**< @brief Permissions associated with the file */

/**
 * @brief           Init the log
 * @return          0 uppon sucess or 1 otherwhise
 */
int init_log();

/**
 * @brief               Write a line to log
 * @param log_action    Description of type of event
 * @param log_info      Additional information about any action
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log(char *log_action, char *log_info);

/**
 * @brief           Close log file
 * @return          0 uppon sucess or 1 otherwhise
 */
int close_log();

#endif // LOG_H_INCLUDED
