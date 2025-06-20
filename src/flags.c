#include "../include/flags.h"

Flags ASSEMBLER_FLAGS = {0};

void PrintHelp() {
    printf("Usage: ./assembler [options] input.as input2.as ...\n");
    printf("Options:\n");
    printf("  -v, --verbose        Enable verbose logging\n");
    printf("  -d, --debug          Enable debug-level logging\n");
    printf("  -q, --quiet          Suppress all logging\n");
    printf("  -s, --symbols        Output symbol table\n");
    printf("  -x, --externals      Output external references\n");
    printf("  -e, --entries        Output entries table\n");
    printf("  -o, --output <file>  Specify output file\n");
    printf("      --version        Show assembler version\n");
    printf("      --help           Show this help message\n");
}

void PrintVersion(void) {
    printf(
        "SNASM: Super-Neat Assembler\n"
        "- Developed and maintained by Dror Sheffer\n"
        "- Built for Super-Neat Assembly (See github for details)\n"
        "- VERSION: (v1.0.0)\n"
    );
}

int ParseFlags(int argc, char **argv, char ***input_files, int *input_count) {
    *input_count = 0;
    *input_files = malloc(argc * sizeof(char*)); // max possible input files

    if (!*input_files) return STATUS_ERROR;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            SetLogLevel(LOG_VERBOSE);
        } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
            SetLogLevel(LOG_DEBUG);
        } else if (strcmp(arg, "-q") == 0 || strcmp(arg, "--quiet") == 0) {
            SetLogLevel(LOG_QUIET);
        } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--symbols") == 0) {
            ASSEMBLER_FLAGS.show_symbols = true;
        } else if (strcmp(arg, "-x") == 0 || strcmp(arg, "--externals") == 0) {
            ASSEMBLER_FLAGS.gen_externals = true;
        } else if (strcmp(arg, "-e") == 0 || strcmp(arg, "--entries") == 0) {
            ASSEMBLER_FLAGS.gen_entries = true;
        } else if (strcmp(arg, "--help") == 0) {
            PrintHelp();
            exit(0);
        } else if (strcmp(arg, "--version") == 0) {
            PrintVersion();
            exit(0);
        } else if ((strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0) && (i + 1 < argc)) {
            ASSEMBLER_FLAGS.output_file = argv[++i];
        } else if (arg[0] == '-') {
            printf("(-) Unknown option: %s\n", arg);
            PrintHelp();
            free(*input_files);
            return STATUS_ERROR;
        } else {
            // Treat as input file
            (*input_files)[(*input_count)++] = strdup(arg);
        }
    }
    
    return 0;
}