#include "../include/command.h"

int DetermineAddressingModes(char *operand,uint8_t opCount);

const Command *FindCommand(char *com_name) {
    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strncmp(com_name, commands[i].name, strlen(commands[i].name)) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

// We shouldnt need to pass the symbol table here, should be a better way
// TODO: Change this
int ValidateCommand(char *com_line, const Command *comm) {
    if (!com_line || !comm) return STATUS_ERROR;

    int offset = 0;
    while (isspace(com_line[offset])) offset++; 
    if (strncmp(com_line, comm->name, strlen(comm->name)) != 0) {
        return STATUS_ERROR;
    }

    int modes = DetermineAddressingModes(com_line + strlen(comm->name), comm->opcount);
    if (modes < 0) {
        printf("Error in validating operands\n");
        return STATUS_ERROR;
    }

    if ((comm->addmodes & modes) != modes) return STATUS_ERROR; // Missmatched addressing
    
    int words = 1;
    int temp = modes;
    while (temp > 0) {
        temp &= (temp - 1);
        words++;
    }
    if (comm->opcount != words - 1) return STATUS_ERROR; // Wrong opcount
    if ((modes & SRC_REG) == SRC_REG) words--;
    if ((modes & DST_REG) == DST_REG) words--;

    return words; // 1 word for command + 1 for each non register operand
}

int DetermineAddressingModes(char *operand, uint8_t opCount) {
    if (!operand || opCount > 2) return STATUS_ERROR;

    uint8_t ret = 0;
    uint8_t ops = 0;
    int offset = 0;

    while (isspace(operand[offset])) offset++;
    
    if (operand[offset] == '\0') return 0; // No operands (e.g., `stop`)

    // Immediate
    if (operand[offset] == '#') {
        offset++;
        if (operand[offset] == NEG_DELIM || operand[offset] == POS_DELIM) offset++;
        if (isdigit(operand[offset])) {
            while (isdigit(operand[offset])) offset++;
        } else return STATUS_ERROR;
        ops++;
        ret |= SRC_IMM;
    }
    // Relative
    else if (operand[offset] == '&') {
        offset++;
        while (operand[offset] && !isspace(operand[offset]) && operand[offset] != ',') offset++;
        ops++;
        ret |= SRC_REL;
    }
    // Register
    else if (operand[offset] == 'r' && operand[offset + 1] >= '0' && operand[offset + 1] <= '7') {
        offset += 2;
        ops++;
        ret |= SRC_REG;
    }
    // Direct
    else {
        while (operand[offset] && !isspace(operand[offset]) && operand[offset] != ',') offset++;
        ops++;
        ret |= SRC_DIR;
    }

    while (isspace(operand[offset])) offset++;

    // Second operand
    if (operand[offset] == ',') {
        offset++;
        while (isspace(operand[offset])) offset++;

        if (operand[offset] == '#') {
            offset++;
            if (operand[offset] == NEG_DELIM || operand[offset] == POS_DELIM) offset++;
            if (isdigit(operand[offset])) {
                while (isdigit(operand[offset])) offset++;
            } else return STATUS_ERROR;
            ops++;
            ret |= DST_IMM;
        }
        else if (operand[offset] == '&') {
            offset++;
            while (operand[offset] && !isspace(operand[offset]) && operand[offset] != ',') offset++;
            ops++;
            ret |= DST_REL;
        }
        else if (operand[offset] == 'r' && operand[offset + 1] >= '0' && operand[offset + 1] <= '7') {
            offset += 2;
            ops++;
            ret |= DST_REG;
        }
        else {
            while (operand[offset] && !isspace(operand[offset]) && operand[offset] != ',') offset++;
            ops++;
            ret |= DST_DIR;
        }
    }

    if (ops != opCount) return STATUS_ERROR;
    if (ops == 1 && ops == opCount) {
        ret <<= 4;
    }

    return ret;
}