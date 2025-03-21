#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/firstpass.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

// Function Prototypes
void CleanAndExit(char **files, size_t files_size);
int PreAssemble(char **files, size_t files_size);
int FirstPass(char **files, size_t files_size);
int GetOutputPath(const char *input_path, char *dst, size_t dst_size, const char *extension);

int main(int argc, char **argv) {
    printf("--- PROGRAM START ---\n");
    
    if (argc < 2) {
        printf("Please input a .as file to be assembled...\n");
        return EXIT_FAILURE;
    }

    // Allocate memory for filenames
    char **files = malloc((argc - 1) * sizeof(char *));
    if (!files) {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        files[i - 1] = strdup(argv[i]);
        if (!files[i - 1]) {
            printf("Error: Failed to allocate memory for file name...\n");
            CleanAndExit(files, i - 1);
            return EXIT_FAILURE;
        }
    }

    // Pre-Assembler Stage
    if (PreAssemble(files, argc - 1) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    // First Pass Stage
    if (FirstPass(files, argc - 1) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    CleanAndExit(files, argc - 1);
    printf("--- PROGRAM END ---\n");
    return EXIT_SUCCESS;
}

// Free allocated memory
void CleanAndExit(char **files, size_t files_size) {
    printf("--- PROGRAM CLEAN ---\n");
    for (size_t i = 0; i < files_size; i++) {
        if (files[i]) free(files[i]);
    }
    free(files);
}

// Pre-Assemble: Expands macros and writes an intermediate .am file
int PreAssemble(char **files, size_t files_size) {
    printf("--- PROGRAM PREASSEMBLE ---\n");

    Macro macros[MAX_MACROS];
    char output_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH];

    for (size_t i = 0; i < files_size; i++) {
        int status = 0;
        size_t count = 0;
        memset(macros, 0, sizeof(macros));

        status = ParseMacros(files[i], macros, &count);
        if (status != 0) {
            printf("Error in macro parsing for file '%s': ParseMacros {%d}\n", files[i], status);
            return status;
        }

        if (GetOutputPath(files[i], output_path, sizeof(output_path), ".am") != 0) {
            printf("Error: Failed to construct output path for file '%s'\n", files[i]);
            return EXIT_FAILURE;
        }

        status = ExpandMacros(files[i], output_path, macros, &count);
        if (status != 0) {
            printf("Error in macro expanding for file '%s': ExpandMacros {%d}\n", files[i], status);
            return status;
        }
    }

    printf("--- PREASSEMBLE SUCCESS ---\n");
    return 0;
}

// First Pass: Builds symbol table and creates .ent file
int FirstPass(char **files, size_t files_size) {
    printf("--- FIRST PASS START ---\n");

    for (size_t i = 0; i < files_size; i++) {
        Label labels[MAX_LABELS] = {0};
        size_t label_count = 0;
        char ent_output[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH];

        if (BuildSymbolTable(files[i], labels, &label_count) != 0) {
            printf("Error in first pass for file '%s'\n", files[i]);
            return EXIT_FAILURE;
        }

        if (GetOutputPath(files[i], ent_output, sizeof(ent_output), ".ent") != 0) {
            printf("Error creating .ent file path for '%s'\n", files[i]);
            continue;
        }

        if (FormatEntryFile(ent_output, labels, &label_count) != 0) {
            printf("Error writing .ent file for '%s'\n", files[i]);
        }
    }

    printf("--- FIRST PASS SUCCESS ---\n");
    return 0;
}

// Constructs the output path with the given extension
int GetOutputPath(const char *input_path, char *dst, size_t dst_size, const char *extension) {
    // Extract the base name
    const char *base_name = strrchr(input_path, '/');
    base_name = (base_name == NULL) ? input_path : base_name + 1;

    // Remove the `.as` extension if it exists
    char trimmed_name[MAX_FILENAME_LENGTH];
    snprintf(trimmed_name, sizeof(trimmed_name), "%s", base_name);

    char *dot = strrchr(trimmed_name, '.');
    if (dot && strcmp(dot, ".as") == 0) {
        *dot = '\0';  // Remove .as extension
    }

    // Construct the output path with the specified extension
    if ((size_t)snprintf(dst, dst_size, "output/%s%s", trimmed_name, extension) >= dst_size) {
        return -1;  // Output path would exceed buffer size
    }

    return 0;
}