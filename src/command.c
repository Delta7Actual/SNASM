#include "../include/command.h"

const Command *FindCommand(char *com_name) {
    if (com_name == NULL) return NULL;
    int l = 0;
    int r = COMMAND_COUNT - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        int cmp = strcmp(com_name, commands[mid].name);
        
        if (cmp == 0) return &commands[mid];
        else if(cmp < 0) r = mid - 1;
        else l = mid + 1;
    }
    return NULL;
}