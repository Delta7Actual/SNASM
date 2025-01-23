#include "../include/macro.h"

Macro *FindMacro(char *name, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (*macro_count == 0 || macro_count == NULL || macros == NULL) return NULL;

    for (size_t i = 0; i < *macro_count; i++) {
        if (strncmp(macros[i].name, name, strlen(name)) == 0) {
            return &macros[i];
        }
    }
    return NULL;
}

int AddMacro(FILE *file_fd, Macro *macro) {
    if (file_fd == NULL || macro == NULL) return STATUS_CATASTROPHIC;

    char line[MAX_LINE_LENGTH] = {0};
    size_t inMacro = 0;
    size_t line_count = 0;
    

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        if (!inMacro) {
            if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0) {
                size_t name_offset = strlen(MACRO_START);
                size_t name_length = 0;

                // Must separate with at least one space
                if (!isblank(line[name_offset])) return STATUS_CATASTROPHIC;
                name_offset++;
                while (isblank(line[name_offset])) name_offset++;
                while (!isblank(line[name_offset + name_length]) && name_length < MAX_MACRO_NAME) name_length++;

                // Copy the macro name into macro->name
                macro->name = strndup(line + name_offset, name_length);
                if (macro->name == NULL) return STATUS_CATASTROPHIC;

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
                if (temp == NULL) return STATUS_CATASTROPHIC;
                macro->body = temp;
                macro->body[line_count] = strndup(line, strlen(line));
                if (macro->body[line_count] == NULL) return STATUS_CATASTROPHIC;
                line_count++;
            }
        }
    }

    // If the loop ends without encountering MACRO_END, it's an error
    return STATUS_NO_RESULT;
}

int CleanUpMacro(Macro *macro) {
    if (macro == NULL) return STATUS_CATASTROPHIC;

    free(macro->name);  // Free macro name
    for (size_t i = 0; i < macro->line_count; i++) {
        free(macro->body[i]);  // Free each line of the body
    }
    free(macro);  // Free the macro structure itself
    return 0;
}