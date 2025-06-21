#ifndef MACRO_H
#define MACRO_H

#include <stddef.h>

#define MAX_MACROS        16
#define MAX_MACRO_NAME    32
#define MAX_MACRO_LINES   128

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "definitions.h"
#include "command.h"


typedef struct s_macro
{
    char *name;
    char **body;
    size_t line_count;
} Macro;

// Returns a pointer to the macro with the given name, or NULL if it doesn't exist
Macro *FindMacro(char *name, Macro macros[MAX_MACROS], size_t *macro_count);

// Returns 0 upon success, ERRORCODE upon failure
int AddMacro(FILE *file_fd, Macro *macro);

// Returns 0 upon success, -1 upon failure
int CleanUpMacro(Macro *macro);

// Returns a pointer to the name
char *GetMacroName(char *line);

#endif