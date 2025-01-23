#include "../include/parser.h"
#include "../include/macro.h"
#include <string.h>
#include <ctype.h>

#define NUM_COMMANDS (sizeof(operations) / sizeof(operations[0]))

// List of operation names
const char *operations[] = {
    "mov", "cmp", "add", "sub", "lea", "clr", 
    "not", "inc", "dec", "jmp", "bne", "jsr", 
    "red", "prn", "rts", "stop"
};

void TrimWhiteSpace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null-terminate the string
    *(end + 1) = '\0';
}

int IsCommandName(char *macro) {

    if (macro == NULL || macro == NULL) {
        return 0; // Not a valid macro name
    }

    TrimWhiteSpace(macro);

    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(macro, operations[i]) == 0) {
            return 1; // Match found
        }
    }
    return 0; // No match found
}

int IsCommentLine(char *line, size_t line_length) {
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == ';') return i;
    }
    return -1;
}
