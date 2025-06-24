#include "../include/preassembler.h"
#include "../include/definitions.h"
#include "../include/secondpass.h"
#include "../include/firstpass.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"

#ifdef _WIN32
#include <direct.h>   // For _mkdir
#else
#include <sys/stat.h> // For mkdir
#include <sys/types.h>
#endif

// Function Prototypes
void CleanAndExit(char **input_files, size_t files_size);
int PreAssemble(char **input_files, size_t files_size);
int FirstPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);
int SecondPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count);

// Constructs the output path with the given extension
int GetOutputPath(const char *input_path, char *dst, size_t dst_size, const char *extension) {
    // Extract the base name
    const char *base_name = strrchr(input_path, '/');
    base_name = (base_name == NULL) ? input_path : base_name + 1;

    // Remove the `.as` extension if it exists
    char trimmed_name[MAX_FILENAME_LENGTH];
    snprintf(trimmed_name, sizeof(trimmed_name), "%s", base_name);

    char *dot = strrchr(trimmed_name, '.');
    if (dot && (strcmp(dot, INPUT_FILE_EXTENSION) == 0 || strcmp(dot, INPUT_FILE_EXTENSION_ALT) == 0)) {
        *dot = '\0';  // Remove .as or .snasm extension
    }

    // Construct the output path with the specified extension
    if ((size_t)snprintf(dst, dst_size, "%s%s", trimmed_name, extension) >= dst_size) {
        return STATUS_ERROR;  // Output path would exceed buffer size
    }

    return 0;
}

bool IsValidSourceFile(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return false;
    return (strcmp(dot, INPUT_FILE_EXTENSION) == 0 || strcmp(dot, INPUT_FILE_EXTENSION_ALT) == 0);
}

char **files = NULL;
int input_count = 0;
const char *output_path = NULL;

int main(int argc, char **argv) {

    // Parse flags
    if (ParseFlags(argc, argv, &files, &input_count) != 0) {
        return 1;
    }

    // Handle -o flag
    if (ASSEMBLER_FLAGS.output_file != NULL) {
        output_path = ASSEMBLER_FLAGS.output_file;
    } else {
        output_path = "out";
    }

    // Check for inputs
    if (input_count == 0) {
        printf("(-) No input files provided.\n");
        PrintHelp();
        return 1;
    }

    // Validate file extensions
    for (int i = 0; i < input_count; i++) {
        if (!IsValidSourceFile(files[i])) {
            printf("(-) Error: Invalid file extension for '%s'. Only '.as' or '.snasm' are allowed.\n", files[i]);
            CleanAndExit(files, input_count);
            return EXIT_FAILURE;
        }
    }

    LogInfo("--- PROGRAM START ---\n");\
    if (ASSEMBLER_FLAGS.legacy_24_bit) LogVerbose("(*) Using legacy 24-bit assembling process...\n");
    if (ASSEMBLER_FLAGS.show_symbols) LogVerbose("(*) Will print symbol table...\n");
    if (ASSEMBLER_FLAGS.gen_externals) LogVerbose("(*) Will generate externals file...\n");

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

    // Cleanup
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
    Macro macros[MAX_MACROS];

    for (size_t i = 0; i < files_size; i++) {
        int status = 0;
        size_t count = 0;
        memset(macros, 0, sizeof(macros));

        status = ParseMacros(input_files[i], macros, &count);
        if (status != 0) {
            printf("(*) Macro parsing for file '%s' failed, Exiting...\n", input_files[i]);
            return status;
        }

        char write_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        if (GetOutputPath(input_files[i], write_path, sizeof(write_path), EXTENDED_FILE_EXTENSION) != 0) {
            printf("(-) Error: Failed to construct output path for file '%s'\n", input_files[i]);
            return EXIT_FAILURE;
        }

        LogVerbose("Successfully generated output path!\n");

        status = ExpandMacros(input_files[i], write_path, macros, &count);
        if (status != 0) {
            printf("(*) Macro expanding for file '%s' failed, Exiting...\n", input_files[i]);
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
        char expanded_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
        if (GetOutputPath(input_files[i], expanded_path, sizeof(expanded_path), EXTENDED_FILE_EXTENSION) != 0) {
            printf("(-) Error: Failed to get expanded path for %s\n", input_files[i]);
            return STATUS_ERROR;
        }
        int status = BuildSymbolTable(expanded_path, labels, label_count);
        if (status != 0) {
            printf("(*) Symbol compilation for file '%s' failed, Exiting...\n", input_files[i]);
            return status;
        }
        LogVerbose("Successfully Pre-Assembled file: %s\n", input_files[i]);
    }

    ICF = IC;
    int symbol_status = ValidateSymbolTable(labels, label_count); 
    if (symbol_status > 0) {
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

    if (symbol_status < 0) { // No entry point found
        printf("(-) Error: Couldn't find entry point to program!\n");
        return STATUS_ERROR;
    }

    LogInfo("--- FIRST PASS SUCCESS ---\n");
    LogVerbose("Current address params IC = %u , DC = %u\n", IC, DC);
    return 0;
}

int SecondPass(char **input_files, size_t files_size, Label labels[MAX_LABELS], size_t *label_count) {

    // Data segment
    uint32_t *data_segment = calloc(DCF+1, sizeof(uint32_t));
    data_segment[0] = 1; // Start from idx = 1

    char write_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};
    char extern_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};

    // Create .sno output path
    if (GetOutputPath(output_path, write_path, sizeof(write_path), OBJECT_FILE_EXTENSION) != 0) {
        printf("(-) Error: could not build .sno output path\n");
        return STATUS_ERROR;
    }
    // Create .sne output path
    if (GetOutputPath(output_path, extern_path, sizeof(extern_path), EXTERNALS_FILE_EXTENSION) != 0) {
        printf("(-) Error: could not build .sne output path\n");
        return STATUS_ERROR;
    }

    LogVerbose("Successfully generated output paths!\n");

    int data_addr = 0;
    for (size_t i = 0; i < files_size; i++) {
        char expanded_path[MAX_FILENAME_LENGTH + MAX_EXTENSION_LENGTH] = {0};

        // Create .snm input path
        if (GetOutputPath(input_files[i], expanded_path, sizeof(expanded_path), EXTENDED_FILE_EXTENSION) != 0) {
            printf("(-) Error: could not build .snm output path\n");
            return STATUS_ERROR;
        }

        int status = EncodeFile(expanded_path, write_path, extern_path, labels, label_count, data_segment, ICF, DCF);
        if (status < 0) {
            printf("(*) Object encoding for file '%s' failed, Exiting...\n", input_files[i]);
            return status;
        }
        data_addr = status;
    }

    FILE *output_fd = fopen(write_path, "a");
    if (!output_fd) {
        printf("(-) Error: could not open output path!\n");
        return STATUS_ERROR;
    }

    data_addr += 100;
    for (uint32_t i = 1; i < data_segment[0]; i++) {
        fprintf(output_fd, "%07u : ", data_addr++);
        WordToHex(output_fd, data_segment[i]);
        LogDebug("Wrote to data segment at %u!\n", data_addr-1);
    }
    fprintf(output_fd, "///\n"); // Seperate object code from entries

    LogVerbose("Text-Section begins at %u, ends at %u\n", 100, ICF -2);
    LogVerbose("Data-Segment begins at %u, ends at %u\n", ICF-1, data_addr-1);
    
    // Re-check labels table
    // entr without extr = warning / extr without entr = error
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].extr && !labels[i].extr_used) {
            printf("(-) Error: Extern label %s was declared but never defined!\n", labels[i].name);
        } else if (labels[i].entr) {
            LogDebug("Note: Entry label %s was declared%s\n",
                labels[i].name,
                labels[i].extr ? " and also marked extern" : "");
                
            // Write entries to output
            fprintf(output_fd, "%s : %07zu\n", labels[i].name, labels[i].address);
            LogDebug("Appended entry label at %zu to output!\n", labels[i].address);
        }
    }
    
    LogInfo("--- SECOND PASS SUCCESS ---\n");
    free(data_segment);
    fclose(output_fd);
    return 0;
}