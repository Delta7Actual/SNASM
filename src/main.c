#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/firstpass.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

void                    CleanAndExit(char **files, size_t files_size);
int                      PreAssemble(char **files, size_t files_size);
int GetOutputPath(const char *input_path, char *dst, size_t dst_size);

int main(int argc, char **argv) {
    printf("--- PROGRAM START ---\n");
    if (argc < 2) {
        printf("Please input a .as file to be assembled...\n");
        return EXIT_FAILURE;
    }

    // Allocate and copy file names
    char **files = malloc((argc - 1) * sizeof(char *));
    if (!files) {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        files[i - 1] = malloc(strlen(argv[i]) + 1); // +1 for null terminator
        if (!files[i - 1]) {
            printf("Error: Failed to allocate memory for file name...\n");
            CleanAndExit(files, i - 1);
            return EXIT_FAILURE;
        }
        strcpy(files[i - 1], argv[i]);
    }

    // Pre-Assemble stage
    if (PreAssemble(files, argc - 1) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    // Future stages...

    CleanAndExit(files, argc - 1);
    printf("--- PROGRAM END ---\n");
    return EXIT_SUCCESS;
}

void CleanAndExit(char **files, size_t files_size) {
    printf("--- PROGRAM CLEAN ---\n");
    for (size_t i = 0; i < files_size; i++) {
        if (files[i]) free(files[i]);
    }

    free(files);
}

int PreAssemble(char **files, size_t files_size) {
    printf("--- PROGRAM PREASSEMBLE ---\n");

    Macro macros[MAX_MACROS];
    char output_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH];

    for (size_t i = 0; i < files_size; i++) {
        int status = 0;
        size_t count = 0;

        memset(macros, 0, sizeof(macros));

        // Parse macros in the file
        status = ParseMacros(files[i], macros, &count);
        if (status != 0) {
            printf("Error in macro parsing for file '%s': ParseMacros {%d}\n", files[i], status);
            printf("--- PREASSEMBLER FAILURE ---\n");
            return status;
        }

        // Get the output path
        if (GetOutputPath(files[i], output_path, sizeof(output_path)) != 0) {
            printf("Error: Failed to construct output path for file '%s'\n", files[i]);
            return EXIT_FAILURE;
        }

        // Expand macros into the output file
        status = ExpandMacros(files[i], output_path, macros, &count);
        if (status != 0) {
            printf("Error in macro expanding for file '%s': ExpandMacros {%d}\n", files[i], status);
            printf("--- PREASSEMBLER FAILURE ---\n");
            return status;
        }
    }

    printf("--- PREASSEMBLE SUCCESS ---\n");
    return 0;
}

int GetOutputPath(const char *input_path, char *dst, size_t dst_size) {
    // Extract the base name
    const char *base_name = strrchr(input_path, '/');
    base_name = (base_name == NULL) ? input_path : base_name + 1;

    // Remove the `.as` extension if it exists
    char trimmed_name[MAX_FILENAME_LENGTH];
    snprintf(trimmed_name, sizeof(trimmed_name), "%s", base_name);

    char *extension = strrchr(trimmed_name, '.');
    if (extension && strcmp(extension, ".as") == 0) {
        *extension = '\0';  // Remove `.as` extension
    }

    // Construct the output path
    if ((size_t)snprintf(dst, dst_size, "output/%s%s", trimmed_name, EXTENDED_FILE_EXTENSION) >= dst_size) {
        return -1;  // Output path would exceed buffer size
    }

    return 0;
}