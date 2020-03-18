#include <utils.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

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

/*----------------------------------------------------------------------------*/
/*                              FILES FUNCTIONS                               */
/*----------------------------------------------------------------------------*/

int path_isdir(const char *path) {
    struct stat status;

    if (stat(path, &status) == -1) {
        return -1;
    }
    return S_ISDIR(status.st_mode);
}
