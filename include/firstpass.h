#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include "definitions.h"
#include "command.h"
#include "label.h"

#include <string.h>
#include <stdlib.h>

extern uint32_t IC;
extern uint32_t DC;
extern uint32_t ICF;
extern uint32_t DCF;

int BuildSymbolTable(char *file_path, Label labels[MAX_LABELS], size_t *label_count);

int ValidateSymbolTable(Label labels[MAX_LABELS], size_t *label_count);

#endif