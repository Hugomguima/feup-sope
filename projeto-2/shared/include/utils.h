#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

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

#define MODE_DELETE     0   /** @brief Mode for trim functions, deletes all trimmed characters */
#define MODE_RMDUP      1   /** @brief Mode for trim functions, deletes all duplicated trimmed characters leaving only one */

/**
 * @brief Trims the string on the right side
 * @param   str         Pointer to string to be trimmed
 * @param   trimmed     Character to trim
 * @param   mode        Trim mode, see macros MODE_DELETE, MODE_RMDUP
 * @return  0 upon sucess, -1 if error occurs
 */
int rtrim(char *str, char trimmed, int mode);

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

/**
 * @brief Concatenates two strings and reallocates space needed
 * @param s1    String that comes first
 * @param s2    String that comes second
 * @param n     Number of characters to concatenate
 * @return Pointer to new string
 */
char* str_cat(char *s1, char *s2, int n);

/*----------------------------------------------------------------------------*/
/*                              FILES FUNCTIONS                               */
/*----------------------------------------------------------------------------*/

int path_isdir(const char *path);

#endif // UTILS_H_INCLUDED
