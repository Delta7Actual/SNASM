#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/// GLOBAL INCLUDES ///
#include <stddef.h>
#include <stdint.h>
#include "logger.h"

/// GLOBAL WINDOWS FUNCTIONS ///
#if defined(_WIN32) || defined(_WIN64)
    #include <string.h>
    #include <stdlib.h>

    char *strndup(const char *s, size_t n);
#endif

/// FILE DEFINITIONS ///
#define INPUT_FILE_EXTENSION       ".as"
#define OBJECT_FILE_EXTENSION      ".ob"
#define EXTENDED_FILE_EXTENSION    ".am"
#define ENTRIES_FILE_EXTENSION     ".ent"
#define EXTERNALS_FILE_EXTENSION   ".ext"

#define MAX_FILENAME_LENGTH        128
#define MAX_EXTENSION_LENGTH       3

/// TESTING DEFINITIONS ///
#ifdef TEST_MODE

#define INPUT_FP                   "tests/input"
#define EXTENDED_OUTPUT_FP         "output/extended"
#define OBJECT_OUTPUT_FP           "output/object"
#define ENTRIES_OUTPUT_FP          "output/entries"
#define EXTERNALS_OUTPUT_FP        "output/externals"

#endif

/// STANDARD INPUT DEFINITIONS ///
#define MAX_LINE_LENGTH   128

// /// OPCODES ///
// #define OPCODE_MOV     0
// #define OPCODE_CMP     1
// #define OPCODE_ADD     2
// #define OPCODE_SUB     3
// #define OPCODE_LEA     4
// #define OPCODE_CLR     5
// #define OPCODE_NOT     6
// #define OPCODE_INC     7
// #define OPCODE_DEC     8
// #define OPCODE_JMP     9
// #define OPCODE_BNE    10
// #define OPCODE_JSR    11
// #define OPCODE_RED    12
// #define OPCODE_PRN    13
// #define OPCODE_RTS    14
// #define OPCODE_STOP   15

/// ADDRESSING MODES ///
#define ADDRESS_IMMEDIATE      0  // #value
#define ADDRESS_DIRECT         1  // label
#define ADDRESS_RELATIVE       2  // &label
#define ADDRESS_REGISTER       3  // rX

/// ERROR CODES ///
#define STATUS_CATASTROPHIC   -1  // Catasrophic error
#define STATUS_NO_RESULT       1  // Failure to complete task, no action needed
#define STATUS_WRONG           2

/// SYMBOL TABLE ///
#define MAX_LABELS           128
#define MAX_LABEL_NAME        31

/// OUTPUT FORMATTING ///
#define WORD_SIZE             24  // Machine word size in bits

/// SPECIAL CHARACTERS ///
#define COMMENT_CHAR   ';'        // For skipping comments
#define LABEL_DELIM    ':'        // For labels
#define MACRO_START    "mcro"     // Start of macro
#define MACRO_END      "mcroend"  // End of macro

/// MACHINE INFORMATION ///
#define MEMORY_SIZE (1 << 21) // 2^21 words


#endif // DEFINITIONS_H