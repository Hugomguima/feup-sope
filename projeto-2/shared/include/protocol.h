#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */

/* Miscellaneous */
#include <pthread.h>

typedef struct request request_t;

/**
 * @brief Request/Reply
 */
typedef struct request {
    int         id;
    pid_t       pid;
    pthread_t   tid;
    int         dur;
    int         pl;
};

/**
 * @brief Fills the request with the arguments given
 *
 * @param request   Pointer to request to be filled
 * @param id        Request ID
 * @param pid       Process ID that made the request
 * @param tid       Thread ID that made the request
 * @param dur       Request duration
 * @return 0 if operation was sucessful, otherwise -1
 */
int fill_request(request_t *request, int id, pid_t pid, pthread_t tid, int dur);

/**
 * @brief Write request to the file
 *
 * @param fd        File descriptor of file
 * @param request   Pointer to request to be written
 * @return  0 if operation was successful, otherwise an error code
 */
int write_request(int fd, const request_t *request);

/**
 * @brief Read request from the file
 *
 * @param fd        File descriptor of file
 * @param request   Pointer to request to be read
 * @return  0 if operation was successful, otherwise an error code
 */
int read_request(int fd, const request_t *request);

/**
 * @brief Write reply to the file
 *
 * @param fd        File descriptor of file
 * @param reply     Pointer to reply to be written
 * @return  0 if operation was successful, otherwise an error code
 */
int write_reply(int fd, const request_t *reply);

/**
 * @brief Read reply from the file
 *
 * @param fd        File descriptor of file
 * @param reply     Pointer to reply to be read
 * @return  0 if operation was successful, otherwise an error code
 */
int read_reply(int fd, const request_t *reply);

/**
 * @brief Fills the reply with the arguments given
 *
 * @param request   Pointer to reply to be filled
 * @param id        Request ID
 * @param pid       Process ID that made the reply
 * @param tid       Thread ID that made the reply
 * @param dur       Duration of the service
 *                  If the service wasn't possible, duration must be set to -1
 *                      Use fill_reply_error to create this type of replies
 * @param pl        Bathroom number assigned
 *                  If the service wasn't possible, assigned number must be set to -1
 *                      Use fill_reply_error to create this type of replies
 * @return 0 if operation was sucessful, otherwise -1
 */
int fill_reply(request_t *reply, int id, pid_t pid, pthread_t tid, int dur, int pl);

/**
 * @brief Fills the reply with the arguments given
 *
 * @param reply     Pointer to reply to be filled
 * @param id        Request ID
 * @param pid       Process ID that made the reply
 * @param tid       Thread ID that made the reply
 * @return 0 if operation was sucessful, otherwise -1
 */
int fill_reply_error(request_t *reply, int id, pid_t pid, pthread_t tid);

#endif /* end of include guard: PROTOCOL_H_INCLUDED */