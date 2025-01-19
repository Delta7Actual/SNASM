#include "macro.h"

// Function for deep copying a macro's name and body to the dst
int copyMacro(Macro *dst, const char *name, char **body, size_t body_size) {
    if (dst == NULL || name == NULL || body == NULL) return 1;

    // Copy the macro name
    dst->name = strdup(name);
    if (!dst->name) {
        return 1;
    }

    // Allocate memory for the body and copy each line
    dst->body = malloc(body_size * sizeof(char *));
    if (!dst->body) {
        free(dst->name);
        return 1;
    }

    for (size_t i = 0; i < body_size; i++) {
        dst->body[i] = strdup(body[i]);
        if (!dst->body[i]) {
            // Cleanup already allocated memory if allocation fails
            for (size_t j = 0; j < i; j++) {
                free(dst->body[j]);
            }
            free(dst->body);
            free(dst->name);
            return 1;
        }
    }

    dst->body_size = body_size;
    return 0; // Successful copy
}

// Returns position in list + 1 if the macro is in the list, 0 otherwise
int isMacroInList(char * name, Macro * macros, size_t count) {

    for (size_t i = 0; i < count; i++) {
        if (strcmp(name, macros[i].name) == 0) {
            return i+1;
        }
    }
    return 0;
}

// Inserts a macro to 'macros' at position 'count' if applicable
// Returns 0 if the macro was inserted successfully, 1 otherwise
int parseMacro(FILE * file, Macro * macros, size_t count) {

    if (file == NULL || macros == NULL) return 1;

    char *name = NULL;
    char **body = NULL;
    size_t body_size = 0;
    int isMacro = 0;
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (!isMacro) {
            // Start of a macro
            if (strncmp(line, "mcro", strlen("mcro")) == 0) {

                int name_offset = 5; // "mcro" + Mandatory space
                while (isblank(line[name_offset])) name_offset++;

                int name_length = 0;
                while (!isblank(line[name_offset + name_length]) && line[name_offset + name_length] != '\n') {
                    name_length++;
                }

                name = (char *)malloc(name_length + 1);
                strncpy(name, line + name_offset, name_length);
                name[name_length] = '\0'; // Null-terminate the string

                if (isMacroInList(name, macros, count) != 0) {
                    // Cleanup
                    free(name);
                    return 1;
                }
                isMacro = 1;
            }
        }
        else {
            // End of a macro
            if (strncmp(line, "mcroend", strlen("mcroend")) == 0) {
                // Use the copyMacro function to store the macro in the array
                if (copyMacro(&macros[count], name, body, body_size) != 0) {
                    // Cleanup
                    free(name);
                    free(body);
                    return 1;
                }

                // Cleanup
                free(name);
                free(body);
                return 0; // Macro was inserted successfully
            }
            else {
                // We are in a macro body
                char **temp = (char **)realloc(body, (body_size + 1) * sizeof(char *)); // Handle realloc failure
                if (temp == NULL) {
                    free(name);
                    free(body);
                    return 1;
                }

                body = temp;
                body[body_size] = strdup(line);
                if (body[body_size] == NULL) {
                    free(name);
                    free(body);
                    return 1;
                }
                body_size++;
            }
        }
    }

    // Cleanup - We found no macro
    free(name);
    free(body);
    return -1; // Macro was not found, we can stop searching
}