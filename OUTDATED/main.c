#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "macro.h"

#include "../definitions.h"

int main(void) {
    FILE *file = fopen("macro.txt", "r");
    FILE *output = fopen("output.txt", "w");
    if (file == NULL || output == NULL) {
        perror("fopen");
        return 1;
    }

    Macro macros[MAX_MACROS]; // Static array for macros
    size_t count = 0;

    while (1) {
        if (count >= MAX_MACROS) {
            printf("Maximum number of macros (%d) reached. Stopping.\n", MAX_MACROS);
            break;
        }

        int result = parseMacro(file, macros, count);

        if (result == 1) { // Catastrophic failure
            printf("Error parsing macro\n");
            break;
        } else if (result == -1) { // No more macros
            printf("No more macros - Current count: %ld\n", count);
            break;
        } else { // Successfully parsed a macro
            printf("Parsed macro\n");
            count++;
        }
    }

    for (size_t i = 0; i < count; i++) {
        fprintf(output, "Macro %ld\n", i);
        fprintf(output, "Name: %s\n", macros[i].name);
        fprintf(output, "Body:\n");
        for (size_t j = 0; j < macros[i].body_size; j++) {
            fprintf(output, "%s\n", macros[i].body[j]);
        }
    }

    // Cleanup
    for (size_t i = 0; i < count; i++) {
        free(macros[i].name);
        for (size_t j = 0; j < macros[i].body_size; j++) {
            free(macros[i].body[j]);
        }
        free(macros[i].body);
    }

    fclose(file);
    fclose(output);
    return 0;
}
