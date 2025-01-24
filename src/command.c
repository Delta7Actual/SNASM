#include "../include/command.h"

Command *FindCommand(char *com_name) {
    for (size_t i = 0; i < commandsSize; i++) {
        if (strncmp(com_name, commands[i].name, strlen(commands[i].name)) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}