/* MAIND HEARDER */
#include "cleanup.h"

/* INCLUDE HEADERS */

/* SYSTEM CALLS HEADERS */

/* C LIBRARY HEADERS */
#include <stdlib.h>

void free_pointers(int n, ...) {
    va_list args;
    va_start(args, n);

    for (int i = 0; i < n; i++) {
        void *p = va_arg(args, void*);
        if (p != NULL) free(p);
    }
    va_end(args);
}
