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
 * @param logString     String to put in log
 * @return              0 uppon sucess or 1 otherwhise
 */
int write_log(char *logString);

/**
 * @brief           Close log file
 * @return          0 uppon sucess or 1 otherwhise
 */
int close_log();

#endif // LOG_H_INCLUDED
