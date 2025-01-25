#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>

#include "definitions.h"

typedef struct s_command {
    const char name[5];
    unsigned short opcode;
    unsigned short funct;
    unsigned short opcount;
} Command;

static const size_t commandsSize = 16;
static const Command commands[] = {
    {"mov",   0, 0, 2},
    {"cmp",   1, 0, 2},
    {"add",   2, 1, 2},
    {"sub",   2, 2, 2},
    {"lea",   4, 0, 2},
    {"clr",   5, 1, 1},
    {"not",   5, 2, 1},
    {"inc",   5, 3, 1},
    {"dec",   5, 4, 1},
    {"jmp",   9, 1, 1},
    {"bne",   9, 2, 1},
    {"jsr",   9, 3, 1},
    {"red",  12, 0, 1},
    {"prn",  13, 0, 1},
    {"rts",  14, 0, 0},
    {"stop", 15, 0, 0}
};

const Command *FindCommand(char *com_name);

#endif