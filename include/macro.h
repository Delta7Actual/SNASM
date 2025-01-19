#ifndef MACRO_H
#define MACRO_H

#include <stddef.h>

#define MAX_MACROS        16
#define MAX_MACRO_NAME    32
#define MAX_MACRO_LINES   128

#include <stddef.h>
#include <stdio.h>
#include "definitions.h"

typedef struct s_macro
{
    char *name;
    char **body;
    size_t line_count;
    size_t size;
} Macro;

// Returns a pointer to the macro with the given name, or NULL if it doesn't exist
Macro *FindMacro(char *name, Macro macros[MAX_MACROS], size_t *macro_count);

// Returns 0 upon success, ERRORCODE upon failure
int AddMacro(FILE *file_fd, Macro *macro);

#endif