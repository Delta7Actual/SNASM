#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>
#include <ctype.h>

#include "symbol.h"
#include "definitions.h"

typedef struct s_command {
    const char *name;
    uint8_t ident;
    uint8_t opcount;
    uint8_t addmodes;
} Command;

#define COMMAND_COUNT 16

#define SRC_IMM (1<<0)
#define SRC_DIR (1<<1)
#define SRC_REL (1<<2)
#define SRC_REG (1<<3)

#define DST_IMM (1<<4)
#define DST_DIR (1<<5)
#define DST_REL (1<<6)
#define DST_REG (1<<7)

#define S_OF(opcode, funct) (((opcode & 0xF) << 4) | (funct & 0xF))
#define G_OP(code) ((code >> 4) & 0xF)
#define G_FT(code) (code & 0xF)

// Command table
static const Command commands[COMMAND_COUNT] = {
    {"mov",  S_OF(0,  0), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"cmp",  S_OF(1,  0), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_IMM | DST_DIR | DST_REG},
    {"add",  S_OF(2,  1), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"sub",  S_OF(2,  2), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"lea",  S_OF(4,  0), 2, SRC_DIR | DST_DIR | DST_REG},
    {"clr",  S_OF(5,  1), 1, DST_DIR | DST_REG},
    {"not",  S_OF(5,  2), 1, DST_DIR | DST_REG},
    {"inc",  S_OF(5,  3), 1, DST_DIR | DST_REG},
    {"dec",  S_OF(5,  4), 1, DST_DIR | DST_REG},
    {"jmp",  S_OF(9,  1), 1, DST_DIR | DST_REL},
    {"bne",  S_OF(9,  2), 1, DST_DIR | DST_REL},
    {"jsr",  S_OF(9,  3), 1, DST_DIR | DST_REL},
    {"red",  S_OF(12, 0), 1, DST_DIR | DST_REG},
    {"prn",  S_OF(13, 0), 1, DST_IMM | DST_DIR | DST_REG},
    {"rts",  S_OF(14, 0), 0, 0}, // No operands
    {"stop", S_OF(15, 0), 0, 0}  // No operands
};

const Command *FindCommand(char *com_name);

// Does not conform to status codes, change?
// Returns number of words the command will take, -1 if error
int ValidateCommand(char *com_line, const Command *comm, Symbol symbols[MAX_LABELS], size_t *symbol_count);

#endif