#ifndef IO_H
#define IO_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"

// Returns number of bytes read from the file, or -1 if an error occurred
int ReadLineFromFile(FILE *file_fd, char *line, size_t line_size);

#endif