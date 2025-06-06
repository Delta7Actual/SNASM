#include "../include/parser.h"

char *TrimWhitespace(char *str) {
    if (!str) return NULL;

    printf("Trimming: %s\n", str);
    
    while (isspace((unsigned char)*str)) str++; // Skip leading spaces
    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--; // Remove trailing spaces
    *(end + 1) = '\0'; // Null terminate

    return str;
}