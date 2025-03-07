#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>

#include "definitions.h"
#include "label.h"

extern const char *operations[] = {
    "mov", "cmp", "add", "sub", "lea", "clr", 
    "not", "inc", "dec", "jmp", "bne", "jsr", 
    "red", "prn", "rts", "stop"
};

typedef enum {
    MOV = 0,
    CMP = 1,
    ADD_SUB = 2,
    LEA = 4,
    INC_DEC_CLR_NOT = 5,
    JMP_BNE_JSR = 9,
    RED = 12,
    PRN = 13,
    RTS = 14,
    STOP = 15
} Opcode;

typedef enum{
    ADD = 1,
    SUB = 2
}FUNCT_2;

typedef enum{
    CLR = 1,
    NOT = 2,
    INC = 3,
    DEC = 4
}FUNCT_5;

typedef enum{
    JMP = 1,
    BNE = 2,
    JSR = 3
}FUNCT_9;

int Mov(int *num1, int *num2);

int Cmp(int *num1, int *num2);

int Add(int *num1, int *num2);

int Sub(int *num1, int *num2);

int Lea(Label *label, int *reg);

int Clr(int *reg);

int Not(int *num);

int Inc(int *num);

int Dec(int *num);

int Jmp(int *address);

int Bne(int *address);

int Jsr(int *address);

int Red(char chr);

int Prn(char chr);

int Rts();

int Stop();

#endif