#include "../include/parser.h"

int IsCommentLine(char *line, size_t line_length) {
    if (line == NULL) return STATUS_CATASTROPHIC;
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == ';') return i;
    }
    return STATUS_CATASTROPHIC;
}