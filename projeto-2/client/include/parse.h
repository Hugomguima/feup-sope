#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#define BIT(n)      (0x1 << (n))    /** @brief Get a mask with bit n activated */

/**
 * @brief Parses command from command line and returns the activated flags
 * @param argc      Number of arguments got from command line
 * @param argv      Pointer to strings containing all the parts from the command
 * @return          1 byte number containing the flags
 */
int parse_cmd(int argc, char *argv[]);

#endif // PARSE_H_INCLUDED
