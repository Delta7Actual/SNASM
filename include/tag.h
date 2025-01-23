#ifndef TAG_H
#define TAG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"


typedef struct {
    char name[50];
    int *address;
    int size;       // Size of the data (number of words)
} Label;

int memory[MAX_TAG_MEMORY];  // Simulated memory
Label labels[MAX_LABELS]; // Labels storage

int label_count = 0;  // Number of labels



#endif