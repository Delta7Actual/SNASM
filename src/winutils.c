#include "../include/definitions.h"

#if defined(_WIN32) || defined(_WIN64)
char *strndup(const char *s, size_t n) {
    size_t len = strnlen(s, n);
    char *result = (char *)malloc(len + 1);
    if (!result) return NULL;
    result[len] = '\0';
    return (char *)memcpy(result, s, len);
}
#endif