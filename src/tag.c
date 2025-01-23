#include <../include/tag.h>



int add_label(char *name, int *data, int size) {
    if (label_count >= MAX_LABELS) {
        return 1;
    }
    Label new_label;
    strncpy(new_label.name, name, sizeof(new_label.name));
    new_label.address = data;
    new_label.size = size;
    labels[label_count++] = new_label;

    return 0;
}
