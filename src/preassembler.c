#include "../include/preassembler.h"
#include "../include/parser.h"

int ExpandMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (file_path == NULL || macros == NULL || macro_count == NULL) return STATUS_CATASTROPHIC;
    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) return STATUS_CATASTROPHIC;

    Macro *curr = malloc(sizeof(Macro));
    if (curr == NULL) {
        fclose(file_fd);
        return STATUS_CATASTROPHIC;
    }
    while (*macro_count < MAX_MACROS) {
        memset(curr, 0, sizeof(*curr));
        int status = AddMacro(file_fd, curr);
        if (status == STATUS_CATASTROPHIC) {
            free(curr);
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        if (status == STATUS_NO_RESULT) break; // No more Macros
        macros[*macro_count] = *curr; // Copy struct
        (*macro_count)++;
    }
    fclose(file_fd);
    free(curr);
    return 0;
}

Macro *DeepCopyMacro(Macro *src) {
    if (src == NULL) return NULL;
    Macro *dst = malloc(sizeof(Macro));
    if (dst == NULL) return NULL;
    memset(dst, 0, sizeof(*dst));

    strncpy(dst->name, src->name, MAX_MACRO_NAME - 1);
    dst->name[MAX_MACRO_NAME - 1] = '\0';
    dst->line_count = src->line_count;

    for (int i = 0; i < src->line_count; i++) {
        strncpy(dst->body[i], src->body[i], MAX_LINE_LENGTH - 1);
        dst->body[i][MAX_LINE_LENGTH - 1] = '\0';
    }
    return dst;
}