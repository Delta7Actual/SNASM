#include "../include/symbol.h"

Symbol *FindSymbol(char *name, Symbol symbols[MAX_LABELS], size_t *symbol_count) {
    if (name == NULL || symbols == NULL || symbol_count == NULL) return NULL;

    for (size_t i = 0; i < symbol_count; i++) {
        if (strncmp(symbols[i].name, name, strlen(symbols[i].name)) == 0) {
            return &symbols[i];
        }
    }
    return NULL;
}



int CleanUpSymbol(Symbol *symbol) {
    if (symbol == NULL) return STATUS_CATASTROPHIC;
    if (symbol->name) free(symbol->name);
    free(symbol);
    return 0;
}