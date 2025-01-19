#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "macro.h"

// Returns 0 upon success, -1 upon failure
int ExpandMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count);

#endif