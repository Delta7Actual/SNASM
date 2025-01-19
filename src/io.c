#include "../include/io.h"

int ReadLineFromFile(FILE* file_fd, char *line, size_t line_size) {
    if (file_fd == NULL || line == NULL) return STATUS_CATASTROPHIC;
    if (fgets(line, MAX_LINE_LENGTH, file_fd) == NULL) return STATUS_CATASTROPHIC;

    int len = strlen(line);
    if (len > 0) line[len - 1] = '\0';
    else return STATUS_CATASTROPHIC;
    return len;
}