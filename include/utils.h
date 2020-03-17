#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

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

#endif // UTILS_H_INCLUDED
