#include "../include/preassembler.h"
#include "../include/definitions.h"
#include "../include/secondpass.h"
#include "../include/firstpass.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

// Function Prototypes
void CleanAndExit(char **files, size_t files_size);
int PreAssemble(char **files, size_t files_size);
int FirstPass(char **files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);
int SecondPass(char **files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);
int GetOutputPath(const char *input_path, char *dst, size_t dst_size, const char *extension);

int main(int argc, char **argv) {
    printf("(+) PROGRAM START\n");

    if (argc < 2) {
        printf("(*) Please input a .as file to be assembled...\n");
        return EXIT_FAILURE;
    }

    // Allocate memory for filenames
    char **files = malloc((argc - 1) * sizeof(char *));
    if (!files) {
        printf("(-) Error: Memory allocation failed");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        files[i - 1] = strdup(argv[i]);
        if (!files[i - 1]) {
            printf("(-) Error: Failed to allocate memory for file name...\n");
            CleanAndExit(files, i - 1);
            return EXIT_FAILURE;
        }
    }

    // Pre-Assembler Stage
    if (PreAssemble(files, argc - 1) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    Label labels[MAX_LABELS] = {0};
    size_t label_count = 0;

    // First Pass Stage
    if (FirstPass(files, argc - 1, labels, &label_count) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    // Second Pass Stage
    if (SecondPass(files, argc - 1, labels, &label_count) != 0) {
        CleanAndExit(files, argc - 1);
        return EXIT_FAILURE;
    }

    CleanAndExit(files, argc - 1);
    printf("(+) PROGRAM END\n");
    return EXIT_SUCCESS;
}

// Free allocated memory
void CleanAndExit(char **files, size_t files_size) {
    printf("(+) PROGRAM CLEAN\n");
    for (size_t i = 0; i < files_size; i++) {
        if (files[i]) free(files[i]);
    }
    free(files);
}

// Pre-Assemble: Expands macros and writes an intermediate .am file
int PreAssemble(char **files, size_t files_size) {
    printf("(+) PROGRAM PREASSEMBLE\n");

    Macro macros[MAX_MACROS];
    char output_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH];

    for (size_t i = 0; i < files_size; i++) {
        int status = 0;
        size_t count = 0;
        memset(macros, 0, sizeof(macros));

        status = ParseMacros(files[i], macros, &count);
        if (status != 0) {
            printf("(-) Error in macro parsing for file '%s': ParseMacros {%d}\n", files[i], status);
            return status;
        }

        if (GetOutputPath(files[i], output_path, sizeof(output_path), ".am") != 0) {
            printf("(-) Error: Failed to construct output path for file '%s'\n", files[i]);
            return EXIT_FAILURE;
        }

        status = ExpandMacros(files[i], output_path, macros, &count);
        if (status != 0) {
            printf("(-) Error in macro expanding for file '%s': ExpandMacros {%d}\n", files[i], status);
            return status;
        }
    }

    printf("(+) PREASSEMBLE SUCCESS\n");
    return 0;
}

// First Pass: Builds symbol table and creates .ent file
int FirstPass(char **files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count) {
    IC = 100;
    printf("(+) FIRST PASS START | IC:%d/DC:%d\n", IC, DC);

    for (size_t i = 0; i < files_size; i++) {
        if (BuildSymbolTable(files[i], labels, label_count) != 0) {
            printf("(-) Error in first pass for file '%s'\n", files[i]);
            return EXIT_FAILURE;
        }
    }

    ICF = IC;
    if (ValidateSymbolTable(labels, label_count) < 0) {
        printf("\n(-) Generated symbol table failed validation!\n\n");
    }
    DCF = DC;
    
    printf("(+) Displaying symbol table\n");
    for (size_t i = 0; i < *label_count; i++) {
        printf("(*) --------------------------------------------------------\n    |Label:%-8s|Addr:%07zu|Entry:%d|Extern:%d|Type:%s|\n",
            labels[i].name,
            labels[i].address,
            labels[i].entr,
            labels[i].extr,
            (labels[i].type == E_CODE) ? "CODE" : "DATA");
        }
        printf("    --------------------------------------------------------\n");
        
    printf("(+) FIRST PASS SUCCESS | IC:%d/DC:%d\n", IC, DC);
    return 0;
}

int SecondPass(char **files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count) {
    printf("(+) SECOND PASS START\n");

    for (size_t i = 0; i < files_size; i++) {
        char output_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        char extern_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};


        // Create .ob output path
        if (GetOutputPath(files[i], output_path, sizeof(output_path), ".ob") != 0) {
            printf("(-) Error: could not build .ob output path\n");
            continue;
        }

        // Create .ext output path
        if (GetOutputPath(files[i], extern_path, sizeof(output_path), ".ext") != 0) {
            printf("(-) Error: could not build .ext output path\n");
            continue;
        }

        printf("(*) Encoding file: %s -> %s\n", files[i], output_path);

        if (EncodeFile(files[i], output_path, extern_path, labels, label_count, ICF, DCF) != 0) {
            printf("(-) Failed to encode %s\n", files[i]);
            continue;
        }

        printf("(+) Encoded %s successfully.\n", files[i]);
    }

    printf("(+) SECOND PASS SUCCESS\n");
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