#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <stddef.h>
#include <ctype.h>

#include "commands.h"
#include "macro.h"
#include "definitions.h"

#define NUM_COMMANDS (sizeof(operations) / sizeof(operations[0]))

// List of operation names

//Returns 1 if true and 0 if not
//Checks if the macro's name is an existing command name
int IsCommandName(char *macro_name);

// Returns index of char where comment begins, or -1 if no comment
// Index begins from zero
int IsCommentLine(char *line, size_t line_length);

// TODO: More to come

#endif