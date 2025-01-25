#include "../include/command.h"

const Command *FindCommand(char *com_name) {
    if (com_name == NULL) return NULL;
    for (size_t i = 0; i < commandsSize; i++) {
        if (strncmp(com_name, commands[i].name, strlen(com_name)) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}