#include "../include/preassembler.h"

/* 
 * (-)
 *  - Opens the given file.
 *  - Reads macros one by one using AddMacro().
 *  - Validates macro names (ensuring they don’t conflict with commands).
 *  - Checks for duplicate macros.
 *  - Stores each found macro in the provided macros array.
 */
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (file_path == NULL || macros == NULL || macro_count == NULL) {
        printf("ParseMacros() received NULL input(s)\n");
        return STATUS_ERROR;
    }

    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) {
        printf("ParseMacros() failed to open file %s\n", file_path);
        return STATUS_ERROR;
    }

    Macro *curr = malloc(sizeof(Macro));
    if (curr == NULL) {
        printf("ParseMacros() failed to allocate memory for Macro\n");
        fclose(file_fd);
        return STATUS_ERROR;
    }

    /* Loop until we reach the maximum number of macros or no more macros are found */
    while (*macro_count < MAX_MACROS) {
        memset(curr, 0, sizeof(*curr));
        int status = AddMacro(file_fd, curr);
        if (status == STATUS_ERROR) {
            printf("(-) Error: AddMacro() failed with status: %d\n", status);
            free(curr);
            fclose(file_fd);
            return STATUS_ERROR;
        }
        if (status == STATUS_NO_RESULT) {
            LogVerbose("Macro parsing complete. Found %zu macros in %s\n", *macro_count, file_path);
            break;  // No more macros found in the file
        }

        /* Remove trailing newline if present */
        TrimNewline(curr->name);
        
        /* Check for duplicate macros */
        if (FindMacro(curr->name, macros, macro_count) != NULL) {
            LogInfo("(-) Error: Found multiple definitions of %s!\n", curr->name);
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
    if (!input_path || !output_path || !macros || !macro_count) {
        printf("ExpandMacros() received NULL input(s)\n");
        return STATUS_ERROR;
    }

    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) {
        LogInfo("ExpandMacros() failed to open input file %s\n", input_path);
        return STATUS_ERROR;
    }

    FILE *output_fd = fopen(output_path, "w");
    if (!output_fd) {
        LogInfo("ExpandMacros() failed to open output file %s\n", output_path);
        fclose(input_fd);
        return STATUS_ERROR;
    }

    char line[MAX_LINE_LENGTH] = {0};
    int in_macro_declaration = 0;

    while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
        // Write empty lines directly
        if (line[0] == '\n' || line[0] == '\r') {
            //fprintf(output_fd, "%s", line);
            LogDebug("Skipping empty line...\n");
            continue;
        }

        // Macro declaration boundaries
        if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0 && isspace(line[strlen(MACRO_START)])) {
            in_macro_declaration = 1;
            continue;
        }
        if (in_macro_declaration && strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
            in_macro_declaration = 0;
            continue;
        }
        if (in_macro_declaration) continue;

        // Handle label + macro call (e.g. START: SETR1)
        char *colon = strchr(line, ':');
        char label_prefix[MAX_LINE_LENGTH] = {0};
        char *macro_candidate = NULL;

        if (colon) {
            size_t label_len = colon - line + 1; // include ':'
            strncpy(label_prefix, line, label_len);
            label_prefix[label_len] = '\0';

            macro_candidate = colon + 1;
            while (isspace(*macro_candidate)) macro_candidate++; // skip spaces
        } else {
            macro_candidate = line;
        }

        // Extract macro name
        char macro_name[MAX_LINE_LENGTH] = {0};
        sscanf(macro_candidate, "%s", macro_name);

        Macro *curr = FindMacro(macro_name, macros, macro_count);

        if (curr) {
            LogDebug("Found macro call for %s\n", macro_name);
            for (size_t i = 0; i < curr->line_count; i++) {
                if (i == 0 && label_prefix[0]) {
                    fprintf(output_fd, "%s%s", label_prefix, curr->body[i]);
                } else {
                    fprintf(output_fd, "%s", curr->body[i]);
                }
                LogDebug("Expanded macro line: %s\n", curr->body[i]);
            }
        } else {
            // Not a macro, write line as-is
            fprintf(output_fd, "%s", line);
            LogDebug("Expanding line...\n");
        }
    }

    fclose(input_fd);
    fclose(output_fd);
    return 0;
}