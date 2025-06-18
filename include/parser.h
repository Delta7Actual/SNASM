#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"

char *TrimWhitespace(char *str);

int HandleDSDirective(char *token, uint32_t *data);

#endif