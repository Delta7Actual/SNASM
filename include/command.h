#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>
#include <ctype.h>


#include "definitions.h"
#include "label.h"

typedef struct s_command {
    const char *name;
    uint16_t   ident;
    uint8_t  opcount;
    uint8_t addmodes;
} Command;

#define COMMAND_COUNT 27

#define SRC_IMM (1<<0)
#define SRC_DIR (1<<1)
#define SRC_REL (1<<2)
#define SRC_REG (1<<3)

#define DST_IMM (1<<4)
#define DST_DIR (1<<5)
#define DST_REL (1<<6)
#define DST_REG (1<<7)

#define S_OF(opcode, funct) (((opcode & 0x3F) << 6) | (funct & 0x3F))
#define G_OP(code) ((code >> 6) & 0x3F)
#define G_FT(code) (code & 0x3F)

// Command table
static const Command commands[COMMAND_COUNT] = {
    // Basic move and arithmetic
    {"mov",  S_OF(0,  0), 2, SRC_IMM | SRC_DIR | SRC_REG | SRC_REL | DST_DIR | DST_REG | SRC_REL},
    {"cmp",  S_OF(1,  0), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_IMM | DST_DIR | DST_REG},
    {"add",  S_OF(2,  1), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"sub",  S_OF(2,  2), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},

    // Memory/address
    {"lea",  S_OF(4,  0), 2, SRC_DIR | DST_DIR | DST_REG},
    {"lod",  S_OF(4,  1), 2, SRC_REG | SRC_IMM | SRC_DIR | DST_REG | DST_DIR},
    {"str",  S_OF(4,  2), 2, SRC_REG | SRC_IMM | SRC_DIR | DST_REG | DST_DIR},

    // Unary operations
    {"clr",  S_OF(5,  1), 1, DST_DIR | DST_REG},
    {"not",  S_OF(5,  2), 1, DST_DIR | DST_REG},
    {"inc",  S_OF(5,  3), 1, DST_DIR | DST_REG},
    {"dec",  S_OF(5,  4), 1, DST_DIR | DST_REG},

    // Branch and subroutine
    {"jmp",  S_OF(9,  1), 1, DST_DIR | DST_REL},
    {"bne",  S_OF(9,  2), 1, DST_DIR | DST_REL},
    {"jsr",  S_OF(9,  3), 1, DST_DIR | DST_REL},

    // Control
    {"rts",  S_OF(14, 0), 0, 0}, // No operands
    {"stop", S_OF(15, 0), 0, 0}, // No operands

    // Logic and extended math
    {"and",  S_OF(2,  3), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"or",   S_OF(2,  4), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"xor",  S_OF(2,  5), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"mul",  S_OF(2,  6), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"div",  S_OF(2,  7), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},
    {"mod",  S_OF(2,  8), 2, SRC_IMM | SRC_DIR | SRC_REG | DST_DIR | DST_REG},

    // Branch and stack
    {"beq",  S_OF(9,  4), 1, DST_DIR | DST_REL},
    {"push", S_OF(10, 0), 1, DST_IMM | DST_DIR | DST_REG},
    {"pop",  S_OF(10, 1), 1, DST_DIR | DST_REG},

    // No-op
    {"nop",  S_OF(15, 1), 0, 0},  // No operands

    // Interrupt
    {"int", S_OF(16, 0), 0, 0} // No operands
};

const Command *FindCommand(char *com_name);

// Does not conform to status codes, change?
// Returns number of words the command will take, -1 if error
int ValidateCommand(char *com_line, const Command *comm);
int DetermineAddressingModes(char *operand, uint8_t opCount);

#endif