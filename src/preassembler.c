#include "../include/preassembler.h"

char CONTEXT[] = "PreAssembler";

int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (file_path == NULL
        || macros == NULL
        || macro_count == NULL) {
            LogError(CONTEXT, "ParseMacros", "Received NULL input");
            return STATUS_CATASTROPHIC;
        };

    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) {
        LogError(CONTEXT, "ParseMacros", "Failed to open file");
        return STATUS_CATASTROPHIC;
    }

    Macro *curr = malloc(sizeof(Macro));
    if (curr == NULL) {
        LogError(CONTEXT, "ParseMacros", "Failed to allocate memory for Macro");
        fclose(file_fd);
        return STATUS_CATASTROPHIC;
    }

    while (*macro_count < MAX_MACROS) {
        memset(curr, 0, sizeof(*curr));
        int status = AddMacro(file_fd, curr);
        if (status == STATUS_CATASTROPHIC) {
            LogError(CONTEXT, "ParseMacros", "AddMacro failed");
            free(curr);
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        if (status == STATUS_NO_RESULT) {
            free(curr);
            fclose(file_fd);
            return 0;
        }
        if (FindCommand(curr->name) != NULL
        || IsCommentLine(curr->name, strlen(curr->name)) > -1) {
            LogError(CONTEXT, "ParseMacros", "Invalid macro name");
            CleanUpMacro(curr);
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        if (status == STATUS_NO_RESULT) break; // No more Macros
        // Macro already exists
        if (FindMacro(curr->name, macros, macro_count) != NULL) {
            CleanUpMacro(curr);
            fclose(file_fd);
            LogError(CONTEXT, "ParseMacros", "Macro already exists");
            return STATUS_CATASTROPHIC;
        }
        macros[*macro_count] = *curr; // Copy struct
        (*macro_count)++;
    }

    fclose(file_fd);
    free(curr);
    return 0;
}

int ExpandMacros(char *input_path, char *output_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (input_path == NULL
        || output_path == NULL
        || macros == NULL
        || macro_count == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Received NULL input");
        return STATUS_CATASTROPHIC;
    }

    FILE *input_fd = fopen(input_path, "r");
    if (input_fd == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Failed to open input file");
        return STATUS_CATASTROPHIC;
    }

    FILE *output_fd = fopen(output_path, "w");
    if (output_fd == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Failed to open output file");
        fclose(input_fd);
        return STATUS_CATASTROPHIC;
    }

    char line[MAX_LINE_LENGTH] = {0};
    int in_macro_declaration = 0;  // Tracks if we're inside a macro declaration

    while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
        // Skip empty lines directly
        if (line[0] == '\n' || line[0] == '\r') {
            if (fprintf(output_fd, "%s", line) < 0) {
                LogError(CONTEXT, "ExpandMacros", "Failed to write empty line to output");
                fclose(input_fd);
                fclose(output_fd);
                return STATUS_CATASTROPHIC;
            }
            continue;
        }

        if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0
        && isspace(line[strlen(MACRO_START)])) {
            in_macro_declaration = 1;
            continue;  // Skip this line
        }
        // Check for the end of a macro declaration
        if (in_macro_declaration && strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
            in_macro_declaration = 0;
            continue;  // Skip this line
        }
        // Skip all lines inside a macro declaration
        if (in_macro_declaration) {
            continue;
        }

        // Skip leading whitespace
        size_t start = strspn(line, " \t");

        // Identify potential macro calls
        size_t name_length = 0;
        while (line[start + name_length] != '\0' && !isspace(line[start + name_length])) {
            name_length++;
        }

        // Extract the first word (potential macro name)
        char *macro_name = strndup(line + start, name_length);
        if (macro_name == NULL) {
            LogError(CONTEXT, "ExpandMacros", "Failed to allocate memory for macro name");
            fclose(input_fd);
            fclose(output_fd);
            return STATUS_CATASTROPHIC;  // Memory allocation failed
        }

        Macro *curr = FindMacro(macro_name, macros, macro_count);
        free(macro_name);  // Free dynamically allocated macro_name

        if (curr != NULL) {
            // Found a macro, write its body to the output
            for (size_t i = 0; i < curr->line_count; i++) {
                if (fprintf(output_fd, "%s", curr->body[i]) < 0) {
                    LogError(CONTEXT, "ExpandMacros", "Failed to write macro body to output");
                    fclose(input_fd);
                    fclose(output_fd);
                    return STATUS_CATASTROPHIC;
                }
            }
        } else {
            // Not a macro, write the line as-is
            if (fprintf(output_fd, "%s", line) < 0) {
                LogError(CONTEXT, "ExpandMacros", "Failed to write line to output");
                fclose(input_fd);
                fclose(output_fd);
                return STATUS_CATASTROPHIC;
            }
        }
    }

    fclose(input_fd);
    fclose(output_fd);
    return 0;
}
