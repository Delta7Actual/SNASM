#include <../include/label.h>




Label* DoesLabelExist(char *name){
    
    for(int i = 0; i < label_count; i++)
        if(strcmp(name , labels[i].name) == 0)
            return &labels[i];

    return NULL;
}

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
