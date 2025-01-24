#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <ctype.h>

#include "definitions.h"
#include "command.h"

// Returns index of char where comment begins, or -1 if no comment
// Index begins from zero
int IsCommentLine(char *line, size_t line_length);

// TODO: More to come

#endif