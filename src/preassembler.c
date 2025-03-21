#include "../include/preassembler.h"

static char CONTEXT[] = "PreAssembler";

/* 
 * ParseMacros:
 *  - Opens the given file.
 *  - Reads macros one by one using AddMacro().
 *  - Validates macro names (ensuring they don’t conflict with commands).
 *  - Checks for duplicate macros.
 *  - Stores each found macro in the provided macros array.
 */
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (file_path == NULL || macros == NULL || macro_count == NULL) {
        LogError(CONTEXT, "ParseMacros", "Received NULL input");
        return STATUS_ERROR;
    }

    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) {
        LogError(CONTEXT, "ParseMacros", "Failed to open file");
        return STATUS_ERROR;
    }

    Macro *curr = malloc(sizeof(Macro));
    if (curr == NULL) {
        LogError(CONTEXT, "ParseMacros", "Failed to allocate memory for Macro");
        fclose(file_fd);
        return STATUS_ERROR;
    }

    /* Loop until we reach the maximum number of macros or no more macros are found */
    while (*macro_count < MAX_MACROS) {
        memset(curr, 0, sizeof(*curr));
        int status = AddMacro(file_fd, curr);
        if (status == STATUS_ERROR) {
            LogError(CONTEXT, "ParseMacros", "AddMacro failed");
            free(curr);
            fclose(file_fd);
            return STATUS_ERROR;
        }
        if (status == STATUS_NO_RESULT) {
            break;  // No more macros found in the file
        }

        /* Remove trailing newline if present */
        size_t name_len = strlen(curr->name);
        if (name_len > 0 && curr->name[name_len - 1] == '\n') {
            curr->name[name_len - 1] = '\0';
        }

        /* Validate macro name: ensure it doesn't conflict with any command */
        if (FindCommand(curr->name) != NULL) {
            LogError(CONTEXT, "ParseMacros", "Invalid macro name: conflicts with a command");
            CleanUpMacro(curr);
            fclose(file_fd);
            return STATUS_ERROR;
        }
        
        /* Check for duplicate macros */
        if (FindMacro(curr->name, macros, macro_count) != NULL) {
            LogError(CONTEXT, "ParseMacros", "Macro already exists");
            CleanUpMacro(curr);
            fclose(file_fd);
            return STATUS_ERROR;
        }
        
        /* Copy the current macro into the macros array */
        macros[*macro_count] = *curr;
        (*macro_count)++;
    }

    fclose(file_fd);
    free(curr);
    return 0;
}

/* 
 * ExpandMacros:
 *  - Opens the input file and output file.
 *  - Scans each line:
 *      - Skips lines within a macro declaration.
 *      - Checks the first word of each line to see if it is a macro call.
 *      - If a macro is found, its body is written to the output.
 *      - Otherwise, the line is copied as-is.
 */
int ExpandMacros(char *input_path, char *output_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (input_path == NULL || output_path == NULL || macros == NULL || macro_count == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Received NULL input");
        return STATUS_ERROR;
    }

    FILE *input_fd = fopen(input_path, "r");
    if (input_fd == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Failed to open input file");
        return STATUS_ERROR;
    }

    FILE *output_fd = fopen(output_path, "w");
    if (output_fd == NULL) {
        LogError(CONTEXT, "ExpandMacros", "Failed to open output file");
        fclose(input_fd);
        return STATUS_ERROR;
    }

    char line[MAX_LINE_LENGTH] = {0};
    int in_macro_declaration = 0;  // Tracks if we're inside a macro declaration

    while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
        /* Write empty lines directly to the output */
        if (line[0] == '\n' || line[0] == '\r') {
            if (fprintf(output_fd, "%s", line) < 0) {
                LogError(CONTEXT, "ExpandMacros", "Failed to write empty line to output");
                fclose(input_fd);
                fclose(output_fd);
                return STATUS_ERROR;
            }
            continue;
        }

        /* Handle macro declaration boundaries */
        if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0 && isspace(line[strlen(MACRO_START)])) {
            in_macro_declaration = 1;
            continue;  // Skip the macro start line
        }
        if (in_macro_declaration && strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
            in_macro_declaration = 0;
            continue;  // Skip the macro end line
        }
        if (in_macro_declaration) {
            continue;  // Skip all lines inside a macro declaration
        }

        /* Skip leading whitespace to extract the first token */
        size_t start = strspn(line, " \t");
        size_t name_length = 0;
        while (line[start + name_length] != '\0' && !isspace(line[start + name_length])) {
            name_length++;
        }

        /* Extract the potential macro name */
        char *macro_name = strndup(line + start, name_length);
        if (macro_name == NULL) {
            LogError(CONTEXT, "ExpandMacros", "Failed to allocate memory for macro name");
            fclose(input_fd);
            fclose(output_fd);
            return STATUS_ERROR;
        }

        Macro *curr = FindMacro(macro_name, macros, macro_count);
        free(macro_name);

        if (curr != NULL) {
            /* Macro call found: write its body to the output */
            for (size_t i = 0; i < curr->line_count; i++) {
                if (fprintf(output_fd, "%s", curr->body[i]) < 0) {
                    LogError(CONTEXT, "ExpandMacros", "Failed to write macro body to output");
                    fclose(input_fd);
                    fclose(output_fd);
                    return STATUS_ERROR;
                }
            }
        } else {
            /* Not a macro call; write the original line as-is */
            if (fprintf(output_fd, "%s", line) < 0) {
                LogError(CONTEXT, "ExpandMacros", "Failed to write line to output");
                fclose(input_fd);
                fclose(output_fd);
                return STATUS_ERROR;
            }
        }
    }

    fclose(input_fd);
    fclose(output_fd);
    return 0;
}