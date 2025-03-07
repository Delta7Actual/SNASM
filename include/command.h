#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>

#include "definitions.h"

typedef struct s_command {
    const char *name;
    uint8_t ident;
    uint8_t opcount;
} Command;

#define COMMAND_COUNT 16

#define S_OF(opcode, funct) (((opcode & 0xF) << 4) | (funct & 0xF))
#define G_OP(code) ((code >> 4) & 0xF)
#define G_FT(code) (code & 0xF)

// Command table
static const Command commands[COMMAND_COUNT] = {
    {"mov",   S_OF(0,  0), 2},
    {"cmp",   S_OF(1,  0), 2},
    {"add",   S_OF(2,  1), 2},
    {"sub",   S_OF(2,  2), 2},
    {"lea",   S_OF(4,  0), 2},
    {"clr",   S_OF(5,  1), 1},
    {"not",   S_OF(5,  2), 1},
    {"inc",   S_OF(5,  3), 1},
    {"dec",   S_OF(5,  4), 1},
    {"jmp",   S_OF(9,  1), 1},
    {"bne",   S_OF(9,  2), 1},
    {"jsr",   S_OF(9,  3), 1},
    {"red",   S_OF(12, 0), 1},
    {"prn",   S_OF(13, 0), 1},
    {"rts",   S_OF(14, 0), 0},
    {"stop",  S_OF(15, 0), 0}
};

const Command *FindCommand(char *com_name);

#endif