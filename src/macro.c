#include "../include/macro.h"

Macro *FindMacro(char *name, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (*macro_count < 0 || macro_count == NULL || macros == NULL) return NULL;
    for (int i = 0; i < *macro_count; i++) {
        if (strncmp(macros[i].name, name, strlen(name)) == 0) {
            return &macros[i];
        }
    }
    return NULL;
}

int AddMacro(FILE *file_fd, Macro *macro) {
    if (file_fd == NULL || macro == NULL) return STATUS_CATASTROPHIC;
    
}