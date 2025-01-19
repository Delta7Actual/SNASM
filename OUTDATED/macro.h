#ifndef MACRO_H
#define MACRO_H

#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../definitions.h"

typedef struct 
{
    char *name;
    char **body;
    size_t body_size;
} Macro;

int parseMacro(FILE *file, Macro * macros, size_t count);

#endif