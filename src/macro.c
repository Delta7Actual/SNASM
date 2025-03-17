#include "../include/macro.h"

Macro *FindMacro(char *name, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (macro_count == NULL || macros == NULL || name == NULL) return NULL;

    for (size_t i = 0; i < *macro_count; i++) {
        if (strncmp(macros[i].name, name, strlen(macros[i].name)) == 0) {
            return &macros[i];
        }
    }
    return NULL;
}

int AddMacro(FILE *file_fd, Macro *macro) {
    if (file_fd == NULL || macro == NULL) return STATUS_ERROR;

    char line[MAX_LINE_LENGTH] = {0};
    size_t inMacro = 0;
    size_t line_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        if (!inMacro) {
            if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0) {
                macro->name = GetMacroName(line);
                if (macro->name == NULL) return STATUS_ERROR;
                inMacro = 1;  // We are now inside a macro
            }
        }
        else {
            if (strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
                macro->line_count = line_count;  // Set the correct line count
                return 0;  // Successfully parsed the macro
            }

            // We are inside the macro
            if (line_count < MAX_MACRO_LINES) {
                // Use strndup to copy line safely
                char **temp = realloc(macro->body, sizeof(line_count + 1) * sizeof(char *));
                if (temp == NULL) return STATUS_ERROR;
                macro->body = temp;
                macro->body[line_count] = strndup(line, strlen(line));
                if (macro->body[line_count] == NULL) return STATUS_ERROR;
                line_count++;
            }
        }
    }

    return STATUS_NO_RESULT;
}

int CleanUpMacro(Macro *macro) {
    if (macro == NULL) return STATUS_ERROR;
    if (macro->name) free(macro->name);  // Free macro name
    for (size_t i = 0; i < macro->line_count; i++) {
        if (macro->body[i]) free(macro->body[i]);  // Free each line of the body
    }
    free(macro);  // Free the macro structure itself
    return 0;
}

char *GetMacroName(char *line) {
    if (line == NULL) return NULL;
    size_t name_offset = strlen(MACRO_START);
    size_t name_length = 0;

    // Must separate with at least one space
    if (!isblank(line[name_offset])) return NULL;
    name_offset++;
    while (isblank(line[name_offset])) name_offset++;
    while (!isblank(line[name_offset + name_length]) && name_length < MAX_MACRO_NAME) name_length++;
    // Copy the macro name into macro->name
    char *ret = strndup(line + name_offset, name_length);
    if (ret == NULL) return NULL;
    return ret;
}