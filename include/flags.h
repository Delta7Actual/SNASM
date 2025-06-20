#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "definitions.h"

typedef struct flags_s {
    bool show_symbols;
    bool gen_externals;
    bool gen_entries;
    const char *output_file;
} Flags;

extern Flags ASSEMBLER_FLAGS;

int ParseFlags(int argc, char **argv, char ***input_files, int *input_count);
void PrintHelp();
void PrintVersion();

#endif