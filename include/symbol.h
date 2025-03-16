#ifndef SYMBOL_H
#define SYMBOL_H

#include <string.h>
#include "definitions.h"

typedef enum e_ltype {
    E_CODE,
    E_DATA,
} LType;

typedef struct s_symbol {
    char *name;
    size_t address;
    LType type;
    uint8_t defined;
    uint8_t entr;
    uint8_t extr;
} Symbol;

Symbol *FindSymbol(char *name, Symbol symbols[MAX_LABELS], size_t *symbol_count);

int AddSymbol(FILE *file_fd, Symbol *symbol);

int CleanUpSymbol(Symbol *symbol);

#endif