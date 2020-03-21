#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

/* INCLUDE HEADERS */

/* SYSTEM CALLS  HEADERS */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* C LIBRARY HEADERS */

/*----------------------------------------------------------------------------*/
/*                              STRING FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

/**
 * @brief Find pattern in str
 * @param   str         Pointer to string to be searched
 * @param   pattern     Pointer to pattern string
 * @param   pos         Starting position
 * @return  Position where substring is present, -1 if error occurs
 */
int str_find(const char *str, const char *pattern, int pos);

/**
 * @brief Splits the string by its delimiters
 * @param   str         Pointer to string to be splitted
 * @param   delim       Pointer to delimiters
 * @return  Array of strings that were splitted, NULL if error occurs
 */
char** str_split(const char *str, const char *delim);

/**
 * @brief Verifies if string composes an integer
 * @param   str         Pointer to string
 * @return  0 if string doesn't compose an integer, 1 if it is an integer, -1 if error occurs
 */
int str_isDigit(const char *str);

/**
 * @brief Verifies if string composes of only alphabetic characters
 * @param   str         Pointer to string
 * @return  0 if string doesn't compose of only alphabetic characters, 1 if it only has alphabetic characters, -1 if error occurs
 */
int str_isAlpha(const char *str);

/*----------------------------------------------------------------------------*/
/*                              FILES FUNCTIONS                               */
/*----------------------------------------------------------------------------*/

typedef enum file_type file_type_t;
/**
 * @brief Enum to store possible file types
 *        File types are obtained using the macros (S_ISREG, S_ISDIR, ...)
 *        FTYPE_ERROR indicates an invalid file type (file doesn't exist)
 *        FTYPE_UNKNOWN indicates a file of unkown type (not labeled in stat)
 */
enum file_type {
    FTYPE_ERROR = -1,
    FTYPE_REG,
    FTYPE_DIR,
    FTYPE_CHR,
    FTYPE_BLOCK,
    FTYPE_FIFO,
    FTYPE_LINK,
    FTYPE_SOCKET,
    FTYPE_UNKNOWN
};

int fget_status(const char *path, struct stat *pstat);

file_type_t fget_type(const char *path);

file_type_t sget_type(const struct stat *pstat);

#endif // UTILS_H_INCLUDED
