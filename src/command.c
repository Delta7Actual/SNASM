#include "../include/command.h"

int DetermineAddressingModes(char *operand,uint8_t opCount);

const Command *FindCommand(char *com_name) {
    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strcmp(com_name, commands[i].name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

int ValidateCommand(char *com_line, const Command *comm) {
    if (!com_line || !comm) return STATUS_ERROR;

    TrimNewline(com_line);
    LogDebug("Validating command %s, Line: %s\n", comm->name, com_line);

    int offset = 0;
    while (isspace(com_line[offset])) offset++; 
    if (strncmp(com_line, comm->name, strlen(comm->name)) != 0) {
        return STATUS_ERROR;
    }

    int modes = DetermineAddressingModes(com_line + strlen(comm->name), comm->opcount);
    if (modes < 0) {
        printf("(-) Error: Illegal operands for command at line: %s\n", com_line);
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

    bool has_reg = false;
    if ((modes & SRC_REG) == SRC_REG) {
        words--;
        has_reg = true;
    }
    if ((modes & DST_REG) == DST_REG && !has_reg) words--;

    LogDebug("Command validated successfully. %d words\n", words);
    return words; // 1 word for command + 1 for each non register operand
}

int DetermineAddressingModes(char *operand, uint8_t opCount) {
    if (!operand || opCount > 2) return STATUS_ERROR;

    uint8_t ret = 0;
    uint8_t ops = 0;
    int offset = 0;

    LogDebug("Determining addressing mode for %u ops: %s\n", opCount, operand);

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
    else if (operand[offset] == 'r') {
        if (operand[offset + 1] >= '0' && operand[offset + 1] <= '7') {
            offset += 2;
            ops++;
            ret |= SRC_REG;
        } else {
            LogDebug("Invalid register found: r%c\n", operand[offset + 1]);
            return STATUS_ERROR;
        }
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
        else if (operand[offset] == 'r') {
            if (operand[offset + 1] >= '0' && operand[offset + 1] <= '7') {
                offset += 2;
                ops++;
                ret |= DST_REG;
            } else {
            LogDebug("Invalid register found: r%c\n", operand[offset + 1]);
            return STATUS_ERROR;
        }
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

    LogDebug("Addressing modes computed: 0x%02X | 0b", ret);
    if (CURRENT_LOG_LEVEL >= LOG_DEBUG) { 
        LogU32AsBin(ret);
    }
    return ret;
}