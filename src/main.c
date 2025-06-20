#include "../include/preassembler.h"
#include "../include/definitions.h"
#include "../include/secondpass.h"
#include "../include/firstpass.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

// Function Prototypes
void CleanAndExit(char **input_files, size_t files_size);
int PreAssemble(char **input_files, size_t files_size);
int FirstPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);
int SecondPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);
int GetOutputPath(const char *input_path, char *dst, size_t dst_size, const char *extension);

char **files = NULL;
int input_count = 0;
const char *output_path = NULL;

int main(int argc, char **argv) {

    if (ParseFlags(argc, argv, &files, &input_count) != 0) {
        return 1;
    }

    if (ASSEMBLER_FLAGS.output_file != NULL) {
        output_path = ASSEMBLER_FLAGS.output_file;
    } else {
        output_path = "out";
    }

    if (input_count == 0) {
        printf("(-) No input files provided.\n");
        PrintHelp();
        return 1;
    }

    LogInfo("--- PROGRAM START ---\n");
    if (ASSEMBLER_FLAGS.show_symbols) LogVerbose("Will print symbol table...\n");
    if (ASSEMBLER_FLAGS.gen_entries) LogVerbose("Will generate entries file...\n");
    if (ASSEMBLER_FLAGS.gen_externals) LogVerbose("Will generate externals file...\n");

    // Pre-Assembler Stage
    if (PreAssemble(files, input_count) != 0) {
        CleanAndExit(files, input_count);
        return EXIT_FAILURE;
    }

    Label labels[MAX_LABELS] = {0};
    size_t label_count = 0;

    // First Pass Stage
    if (FirstPass(files, input_count, labels, &label_count) != 0) {
        CleanAndExit(files, input_count);
        return EXIT_FAILURE;
    }

    // Second Pass Stage
    if (SecondPass(files, input_count, labels, &label_count) != 0) {
        CleanAndExit(files, input_count);
        return EXIT_FAILURE;
    }

    CleanAndExit(files, input_count);
    LogInfo("--- PROGRAM END ---\n");
    return EXIT_SUCCESS;
}

// Free allocated memory
void CleanAndExit(char **input_files, size_t files_size) {
    LogInfo("--- PROGRAM CLEAN ---\n");
    for (size_t i = 0; i < files_size; i++) {
        if (input_files[i]) free(input_files[i]);
    }
    free(input_files);
}

// Pre-Assemble: Expands macros and writes an intermediate .am file
int PreAssemble(char **input_files, size_t files_size) {
    LogVerbose("--- PROGRAM PREASSEMBLE ---\n");

    Macro macros[MAX_MACROS];

    for (size_t i = 0; i < files_size; i++) {
        int status = 0;
        size_t count = 0;
        memset(macros, 0, sizeof(macros));

        status = ParseMacros(input_files[i], macros, &count);
        if (status != 0) {
            printf("(-) Error in macro parsing for file '%s': ParseMacros {%d}\n", input_files[i], status);
            return status;
        }

        char write_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        if (GetOutputPath(input_files[i], write_path, sizeof(write_path), ".am") != 0) {
            printf("(-) Error: Failed to construct output path for file '%s'\n", input_files[i]);
            return EXIT_FAILURE;
        }

        LogVerbose("Successfully generated output path!\n");

        status = ExpandMacros(input_files[i], write_path, macros, &count);
        if (status != 0) {
            printf("(-) Error in macro expanding for file '%s': ExpandMacros {%d}\n", input_files[i], status);
            return status;
        }

        LogVerbose("Successfully Pre-Assembled file: %s\n", input_files[i]);
    }

    LogInfo("--- PREASSEMBLE SUCCESS ---\n");
    return 0;
}

// First Pass: Builds symbol table and creates .ent file
int FirstPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count) {
    IC = 100;
    LogDebug("Starting address params: IC = %u | DC = %u\n", IC, DC);

    for (size_t i = 0; i < files_size; i++) {
        if (BuildSymbolTable(input_files[i], labels, label_count) != 0) {
            printf("(-) Error in first pass for file '%s'\n", input_files[i]);
            return EXIT_FAILURE;
        }
        LogVerbose("Successfully Pre-Assembled file: %s\n", input_files[i]);
    }

    ICF = IC;
    int symbol_status = ValidateSymbolTable(labels, label_count); 
    if (symbol_status != 0) {
        LogDebug("Warning: Found %u warnings in symbol validation, will re-check in second pass...\n", symbol_status);
    }
    DCF = DC;
    
    if (ASSEMBLER_FLAGS.show_symbols > 0) {
    printf("Displaying symbol table\n");
        for (size_t i = 0; i < *label_count; i++) {
            printf("    ----------------------------------------------------------------------\n    |Label:%-8s|Addr:%07zu|Entry:%d|Extern:%d|Extern Used:%d|Type:%s|\n",
                labels[i].name,
                labels[i].address,
                labels[i].entr,
                labels[i].extr,
                labels[i].extr_used,
                (labels[i].type == E_CODE) ? "CODE" : "DATA");
            }
        printf("    ----------------------------------------------------------------------\n");
    }

    LogInfo("--- FIRST PASS SUCCESS ---\n");
    LogVerbose("Current address params IC = %u , DC = %u\n", IC, DC);
    return 0;
}

int SecondPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count) {
    for (size_t i = 0; i < files_size; i++) {
        char write_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        char extern_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        char entry_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};

        // Create .ob output path
        if (GetOutputPath(output_path, write_path, sizeof(write_path), ".ob") != 0) {
            printf("(-) Error: could not build .ob output path\n");
            continue;
        }

        // Create .ext output path
        if (GetOutputPath(output_path, extern_path, sizeof(extern_path), ".ext") != 0) {
            printf("(-) Error: could not build .ext output path\n");
            continue;
        }

        // Create .ent output path
        if (GetOutputPath(output_path, entry_path, sizeof(entry_path), ".ent") != 0) {
            printf("(-) Error: could not build .ent output path\n");
            continue;
        }

        LogVerbose("Successfully generated output paths!\n");

        if (EncodeFile(input_files[i], write_path, extern_path, entry_path, labels, label_count, ICF, DCF) != 0) {
            printf("(-) Failed to encode %s\n", input_files[i]);
            continue;
        }
    }

    // Re-check labels table
    // entr without extr = warning / extr without entr = error
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].extr && !labels[i].extr_used) {
            printf("(*) Error: Extern label %s was declared but never used!\n", labels[i].name);
        } else if (labels[i].entr) {
            LogDebug("Note: Entry label %s was declared%s\n",
                    labels[i].name,
                    labels[i].extr ? " and also marked extern" : "");
        }
    }

    LogInfo("--- SECOND PASS SUCCESS ---\n");
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
        return STATUS_ERROR;  // Output path would exceed buffer size
    }

    return 0;
}