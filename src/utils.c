/* MAIN HEADER */
#include "utils.h"

/* INCLUDE HEADERS */

/* SYSTEM CALLS  HEADERS */

/* C LIBRARY HEADERS */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

//temp
#include <stdlib.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/
/*                              STRING FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

int str_find(const char *str, const char *pattern, int pos) {
    if (str == NULL || pattern == NULL) return -1;

    int len1 = strlen(str);
    int len2 = strlen(pattern);

    if (len1 < len2 || pos >= len1 || len1 == 0 || len2 == 0) return -1;

    for (int i = pos; i <= len1 - len2; i++) {
        int j = 0;
        while (j < len2 && str[i + j] == pattern[j]) j++;

        if (j == len2) return i;
    }

    return -1;
}

int rtrim(char *str, char trimmed, int mode) {
    if (str == NULL || trimmed == 0) return -1;
    if (mode != MODE_DELETE && mode != MODE_RMDUP) return -1;

    int end = strlen(str) - 1;

    while (end >= 0 && str[end] == trimmed) end--;

    if (mode == MODE_RMDUP && end != (int)strlen(str) - 1) {
        str[end + 2] = 0;
    } else {
        str[end + 1] = 0;
    }
    return 0;
}

char** str_split(const char *str, const char *delim) {
    if (str == NULL || delim == NULL) return NULL;

    char **result = NULL;
    int count = 0;
    int pos = 0;
    char *dup = strdup(str);

    while ((pos = str_find(dup, delim, pos)) != -1) { // while there's matches
        count++;
    }

    count += 2; // add one for last group and one for NULL pointer

    result = (char**)malloc(sizeof(char*) * count);

    if (result != NULL) {
        int index = 0;
        char *token = strtok(dup, delim);

        while (token) {
            assert(index < count);
            *(result + index++) = strdup(token);
            token = strtok(NULL, delim);
        }

        assert(index == count - 1);
        *(result + index) = NULL;
    }

    return result;
}

int str_isDigit(const char *str) {
    if (str == NULL || strlen(str) == 0) return -1;

    int i = 0;
    char ch;
    while ((ch = str[i++]) != 0)
        if (ch < '0' || ch > '9') return 0;

    return 1;
}

int str_isAlpha(const char *str) {
    if (str == NULL || strlen(str) == 0) return -1;

    int i = 0;
    char ch;
    while ((ch = str[i++]) != 0)
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))) return 0;

    return 1;
}

char* str_cat(char *s1, char *s2, int n) {
    int len1 = (int)strlen(s1);
    int len2 = (n < (int)strlen(s2)) ? n : (int)strlen(s2);

    char *res = (char*)malloc(sizeof(char) * (len1 + len2 + 1));

    if (res == NULL) {
        char *error = strerror(errno);
        write(STDERR_FILENO, error, strlen(error));
        write(STDERR_FILENO, "\n", 1);
        return NULL;
    }

    res = strcpy(res, s1);

    if (res == NULL) {
        char *error = strerror(errno);
        write(STDERR_FILENO, error, strlen(error));
        write(STDERR_FILENO, "\n", 1);
        return NULL;
    }

    return strncat(res, s2, len2);
}

/*----------------------------------------------------------------------------*/
/*                              FILES FUNCTIONS                               */
/*----------------------------------------------------------------------------*/

int fget_status(const char *path, struct stat *pstat, int deref_sym) {
    if(deref_sym) { // If -L is set, then dereference the symbolic link
        if (stat(path, pstat) == -1) {
              printf("here");
              char *error = strerror(errno);
              write(STDERR_FILENO, error, strlen(error));
              write(STDERR_FILENO, "\n", 1);
              return -1;
        }
    }
    else {
        if (lstat(path, pstat) == -1) {
              char *error = strerror(errno);
              write(STDERR_FILENO, error, strlen(error));
              write(STDERR_FILENO, "\n", 1);
              return -1;
        }
    }
    return 0;
}

file_type_t fget_type(const char *path, int deref_sym) {
    struct stat status;

    if (fget_status(path, &status, deref_sym)) {
        return FTYPE_ERROR;
    }

    return sget_type(&status);
}

file_type_t sget_type(const struct stat *pstat) {
    switch (pstat->st_mode & S_IFMT) {
        case S_IFREG:
            return FTYPE_REG;
        case S_IFDIR:
            return FTYPE_DIR;
        case S_IFCHR:
            return FTYPE_CHR;
        case S_IFBLK:
            return FTYPE_BLOCK;
        case S_IFLNK:
            return FTYPE_LINK;
        case S_IFIFO:
            return FTYPE_FIFO;
        case S_IFSOCK:
            return FTYPE_SOCKET;
        default:
            return FTYPE_UNKNOWN;
    }
}

double fget_size(int bytes, struct stat *status, int block_size) {
    double fsize;
    if (bytes) {
        fsize = status->st_size;
    } else {
        if (block_size > 512) {
            fsize = (status->st_blocks) / (block_size / 512.0);
        } else {
            fsize = (status->st_blocks) / ((double) block_size);
        }
    }
    return fsize;
}

/*----------------------------------------------------------------------------*/
/*                              MATH FUNCTIONS                                */
/*----------------------------------------------------------------------------*/

long dceill(double x) {
    return ((x - (long)x) > 0) ? (long)(x + 1) : (long)x;
}
