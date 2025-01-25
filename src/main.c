#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

int main(void) {
    printf("Starting program...\n");

    Macro macros[MAX_MACROS] = {0};
    size_t count = 0;
    printf("Parsing macros...\n");
    int result = ParseMacros("test/test1.asm", macros, &count);
    if (result != 0) {
        printf("ERROR: ParseMacros returned %d\n", result);
        return 1;
    }
    printf("Macro parsing complete. Count: %zu\n", count);
    printf("----------\n");
    for (size_t i = 0; i < count; i++) {
        printf("Macro %zu: %s", i, macros[i].name);
        for (size_t j = 0; j < macros[i].line_count; j++) {
            printf("%s", macros[i].body[j]);
        }
    }
    printf("----------\n");
    result = ExpandMacros("test/test1.asm", "test/test2.asm", macros, &count);
    
    if (result != 0) printf("ERROR at macro expansion");

    printf("Program finished.\n");
    return EXIT_SUCCESS;
}