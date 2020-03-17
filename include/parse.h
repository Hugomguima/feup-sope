#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#define BIT(n)      (0x1 << (n))    /** @brief Get a mask with bit n activated */

// simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]

// -l, --count-links
#define FLAG_LINKS      BIT(0)  /** @brief Count the same file multiple times */
// -a, --all
#define FLAG_ALL        BIT(1)  /** @brief The displayed information also concerns files */
// -b, --bytes
#define FLAG_BYTES      BIT(2)  /** @brief Displays the actual number of data bytes (files) or allocated (directories) */
// -B size, --block-size=SIZE
#define FLAG_BSIZE      BIT(3)  /** @brief Defines the size (bytes) of the block for representation purposes */
// -L, --dereference
#define FLAG_DEREF      BIT(4)  /** @brief Follow symbolic links */
// -S, --separate-dirs
#define FLAG_SEPDIR     BIT(5)  /** @brief The displayed information does not include the size of the subdirectories */
// --max-depth=N
#define FLAG_MAXDEPTH   BIT(6)  /** @brief Limits the displayed information to N (0.1, ...) levels of directory depth */
// custom path
#define FLAG_PATH       BIT(7)  /** @brief Use custom path */
// Flag error
#define FLAG_ERR        BIT(8)  /** @brief Error flag */

/**
 * @brief Parses command from command line and returns the activated flags
 * @param argc      Number of arguments got from command line
 * @param argv      Pointer to strings containing all the parts from the command
 * @return          1 byte number containing the flags
 */
int parse_cmd(int argc, char *argv[]);

#endif // PARSE_H_INCLUDED
