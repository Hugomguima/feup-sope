#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

/**
 * @brief           Init the log
 * @return          0 uppon sucess or 1 otherwhise
 */
int init_log();

/**
 * @brief           Write a line to log
 * @return          0 uppon sucess or 1 otherwhise
 */
int write_log();

/**
 * @brief           Close log file
 * @return          0 uppon sucess or 1 otherwhise
 */
int close_log();

#endif // LOG_H_INCLUDED
