#include "../include/parser.h"

int IsCommentLine(char *line, size_t line_length) {
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == ';') return i;
    }
    return -1;
}