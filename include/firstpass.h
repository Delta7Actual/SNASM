#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include "definitions.h"
#include "command.h"
#include "label.h"
#include "parser.h"

#include <string.h>
#include <stdlib.h>

uint32_t IC, DC, ICF, DCF;

int BuildSymbolTable(char *file_path, Label labels[MAX_LABELS], size_t *label_count);

int FormatEntryFile(char *file_name, Label labels[MAX_LABELS], size_t *label_count);

#endif