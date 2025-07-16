#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "definitions.h"

typedef struct flags_s {
    bool start_exists;
    bool show_symbols;
    // bool gen_entries;
    bool gen_externals;
    const char *output_file;
    bool entry_point_exists;
    bool append_to_out;
    // bool append_to_ent;
    // bool append_to_ext;
    bool legacy_24_bit;
} Flags;

extern Flags ASSEMBLER_FLAGS;

int ParseFlags(int argc, char **argv, char ***input_files, int *input_count);
void PrintHelp();
void PrintVersion();

#endif