#include "../include/symbol.h"

Label *FindLabel(char *name, Label labels[MAX_LABELS], size_t *label_count) {
    if (name == NULL || labels == NULL || label_count == NULL) return NULL;

    for (size_t i = 0; i < *label_count; i++) {
        if (strncmp(labels[i].name, name, strlen(labels[i].name)) == 0) {
            return &labels[i];
        }
    }
    return NULL;
}



int CleanUpLabel(Label *label) {
    if (label == NULL) return STATUS_CATASTROPHIC;
    if (label->name) free(label->name);
    free(label);
    return 0;
}