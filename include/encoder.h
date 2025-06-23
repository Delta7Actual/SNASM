#ifndef ENCODER_H
#define ENCODER_H

#include "definitions.h"
#include "command.h"
#include "label.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define M (1 << 3)
#define A (1 << 2)
#define R (1 << 1)
#define E (1 << 0)

int EncodeCommand(char *ops, const Command *comm, uint8_t modes, uint32_t *out);

uint32_t EncodeImm(char *op, bool is_last);
uint32_t EncodeDir(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, FILE *extern_fd, bool is_last);
uint32_t EncodeRel(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, FILE *extern_fd, bool is_last);

void WordToHex(FILE *file, uint32_t num);
void LogU32AsBin(uint32_t num);

#endif