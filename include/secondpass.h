#ifndef SECONDPASS_H
#define SECONDPASS_H

#include "definitions.h"
#include "encoder.h"
#include "parser.h"
#include "label.h"

int EncodeFile(char *input_path, char *output_path, Label *labels, size_t *label_count, uint32_t *data_segment, uint32_t icf, uint32_t dcf);

#endif