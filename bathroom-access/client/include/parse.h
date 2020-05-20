#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#define BIT(n)      (0x1 << (n))    /** @brief Get a mask with bit n activated */

// Qn -t nsecs [-l nplaces] [-n nthreads] fifoname

// -t nsecs
#define FLAG_SECS       BIT(0)  /** @brief Program execution duration */
// fifoname
#define FLAG_FIFO       BIT(1)  /** @brief Name of the public FIFO to fulfill requests */
// Flag error
#define FLAG_ERR        BIT(3)  /** @brief Error flag */

typedef struct parse_info parse_info_t;
/**
 * @brief Information to be filled in parse_cmd
 */
struct parse_info {
    char      *path;
    int       exec_secs;
};

void init_parse_info(parse_info_t *info);

void free_parse_info(parse_info_t *info);

/**
 * @brief Parses command from command line and returns the activated flags
 * @param argc      Number of arguments got from command line
 * @param argv      Pointer to strings containing all the parts from the command
 * @param info      Pointer to struct parse_info_t
                    Memory for path is allocated in function and should be freed
 * @return          1 byte number containing the flags
 */
int parse_cmd(int argc, char *argv[], parse_info_t *info);

#endif // PARSE_H_INCLUDED
