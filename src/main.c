#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

int main(void) {
    printf("Starting program...\n");

    Macro macros[MAX_MACROS] = {0};
    size_t count = 0;
    printf("Expanding macros...\n");
    int result = ParseMacros("test/test1.asm", macros, &count);
    if (result != 0) {
        printf("ERROR: ExpandMacros returned %d\n", result);
        return 1;
    }
    printf("Macro expansion complete. Count: %zu\n", count);
    printf("----------\n");
    for (size_t i = 0; i < count; i++) {
        printf("Macro %zu: %s\n", i, macros[i].name);
    }

    printf("Program finished.\n");
    return EXIT_SUCCESS;
}