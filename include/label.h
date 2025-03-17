#ifndef SYMBOL_H
#define SYMBOL_H

#include <string.h>
#include <ctype.h>
#include "definitions.h"
#include "parser.h"

typedef enum e_ltype {
    E_CODE,
    E_DATA,
} LType;

typedef struct s_symbol {
    char *name;
    size_t address;
    LType type;
    uint8_t entr;
    uint8_t extr;
} Label;

Label *FindLabel(char *name, Label labels[MAX_LABELS], size_t *label_count);

int AddLabel(FILE *file_fd, Label *symbol);

int ValidLabelName(char *name);

int CleanUpLabel(Label *symbol);

#endif