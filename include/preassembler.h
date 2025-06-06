#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "macro.h"
#include "parser.h"
#include "command.h"

// Returns 0 upon success, else ERRORCODE
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count);

// Returns 0 upon success, else ERRORCODE
int ExpandMacros(char *input_path, char *output_path, Macro macros[MAX_MACROS], size_t *macro_count);

#endif