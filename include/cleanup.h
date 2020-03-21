#ifndef CLEANUP_H_INCLUDED
#define CLEANUP_H_INCLUDED

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */

/* C LIBRARY HEADERS */
#include <stdarg.h>

/**
 * @brief Free memory for all pointers passed
 * @param   n       Number of pointers passed to the function
 * @param   ...     Pointers to be freed
 */
void free_pointers(int n, ...);

#endif // CLEANUP_H_INCLUDED
