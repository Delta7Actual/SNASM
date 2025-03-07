#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/// FILE DEFINITIONS ///
#define INPUT_FILE_EXTENSION       ".as"
#define OBJECT_FILE_EXTENSION      ".ob"
#define EXTENDED_FILE_EXTENSION    ".am"
#define ENTRIES_FILE_EXTENSION     ".ent"
#define EXTERNALS_FILE_EXTENSION   ".ext"

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

/// OPCODES ///
#define OPCODE_MOV     0
#define OPCODE_CMP     1
#define OPCODE_ADD     2
#define OPCODE_SUB     2
#define OPCODE_LEA     4
#define OPCODE_CLR     5
#define OPCODE_NOT     5
#define OPCODE_INC     5
#define OPCODE_DEC     5
#define OPCODE_JMP     9
#define OPCODE_BNE     9
#define OPCODE_JSR     9
#define OPCODE_RED    12
#define OPCODE_PRN    13
#define OPCODE_RTS    14
#define OPCODE_STOP   15

/// FUNCT ///
#define FUNCT_ADD      1
#define FUNCT_SUB      2
#define FUNCT_CLR      1
#define FUNCT_NOT      2
#define FUNCT_INC      3
#define FUNCT_DEC      4
#define FUNCT_JMP      1
#define FUNCT_BNE      2
#define FUNCT_JSR      3

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
#define MAX_SYMBOLS            128
#define MAX_SYMBOL_NAME        32

/// OUTPUT FORMATTING ///
#define WORD_SIZE              24 // Machine word size in bits
#define OUTPUT_BASE            6  // Default output base (e.g., base 10)

/// SPECIAL CHARACTERS ///
#define COMMENT_CHAR   ';'       // For skipping comments
#define LABEL_DELIM    ':'       // For labels
#define MACRO_START    "mcro"    // Start of macro
#define MACRO_END      "mcroend" // End of macro

///MEMORY LIMITS///
#define MAX_TAG_MEMORY 1024

///TAGS///
#define MAX_LABELS 128

///REGISTERS///
#define MAX_REGISTER 8


#endif // DEFINITIONS_H
