#include "../include/command.h"

uint8_t DetermineAddressingModes(char *operand, Label labels[MAX_LABELS], size_t *label_count);

const Command *FindCommand(char *com_name) {
    if (com_name == NULL) return NULL;

    printf("FC:%s\n", com_name);

    int l = 0;
    int r = COMMAND_COUNT - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        // Just compare relevant characters
        int cmp = strncmp(com_name, commands[mid].name, strlen(commands[mid].name));
        
        if (cmp == 0) return &commands[mid];
        else if(cmp < 0) r = mid - 1;
        else l = mid + 1;
    }
    return NULL;
}

// We shouldnt need to pass the symbol table here, should be a better way
// TODO: Change this
int ValidateCommand(char *com_line, const Command *comm, Label labels[MAX_LABELS], size_t *label_count) {
    if (!com_line || !comm) return STATUS_ERROR;

    printf("VC:%s\n", com_line);

    int offset = 0;
    while (isspace(com_line[offset])) offset++; 
    if (strncmp(com_line, comm->name, strlen(comm->name)) != 0) return STATUS_ERROR;
    
    while (isspace(com_line[offset])) offset++;

    uint8_t modes = DetermineAddressingModes(com_line, labels, label_count);
    if ((comm->addmodes & modes) != modes) return STATUS_ERROR; // Missmatched addressing
    
    int words = 1;
    uint8_t temp = modes;
    while (temp > 0) {
        temp &= (temp - 1);
        temp++;
    }
    if (comm->opcount != words - 1) return STATUS_ERROR; // Wrong opcount
    if ((modes & SRC_REG) == SRC_REG) words--;
    if ((modes & DST_REG) == DST_REG) words--;

    return words; // 1 word for command + 1 for each non register operand
}

// Returns -1 if syntax error
uint8_t DetermineAddressingModes(char *operand, Label labels[MAX_LABELS], size_t *label_count) {
    if (!operand || !labels || !label_count) return STATUS_ERROR;

    printf("OP:%s\n", operand);

    uint8_t ret = 0;
    int offset = 0;

    // Immediate
    if (operand[0] == '#') {
        offset++;
        while (operand[offset]) {
            if (!isdigit(operand[offset]) && operand[offset] != ',') return STATUS_ERROR;
            if (isdigit(operand[offset])) offset++;
            if (operand[offset] == ',') {
                ret |= SRC_IMM;
                break;
            }
        }
    }
    // Relative
    else if (operand[0] == '&') {
        offset++;
        Label *ref = FindLabel(operand, labels, label_count);
        if (!ref) return STATUS_ERROR;
        if (ref->extr > 0) return STATUS_ERROR;
        offset += strlen(ref->name);
        ret |= SRC_REL;
    }
    // Register (This one's nice I like this one :D)
    else if (operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7') {
        offset += 2;
        ret |= SRC_REG;
    }
    // Either Direct or illegal
    else {
        Label *ref = FindLabel(operand + offset, labels, label_count);
        if (!ref) return STATUS_ERROR;
        if (ref->type != E_DATA && ref->extr == 0) return STATUS_ERROR;
        offset += strlen(ref->name);
        ret |= SRC_DIR;
    }

    // Second exists -> repeat checks
    if (operand[offset] == ',') {
        offset++;
        // Immediate
        if (operand[offset] == '#') {
            offset++;
            while (operand[offset]) {
                if (!isdigit(operand[offset]) && operand[offset] != ',') return STATUS_ERROR;
                if (isdigit(operand[offset])) offset++;
                if (operand[offset] == ',') {
                    ret |= DST_IMM;
                    break;
                }
            }
        }
        // Relative
        else if (operand[offset] == '&') {
            offset++;
            Label *ref = FindLabel(operand + offset, labels, label_count);
            if (!ref) return STATUS_ERROR;
            if (ref->extr > 0) return STATUS_ERROR;
            offset += strlen(ref->name);
            ret |= DST_REL;
        }
        // Register (This one's nice I like this one :D)
        else if (operand[offset] == 'r' && operand[offset+1] >= '0' && operand[offset+1] <= '7') {
            offset += 2;
            ret |= DST_REG;
        }
        // Either Direct or illegal
        else {
            Label *ref = FindLabel(operand, labels, label_count);
            if (!ref) return STATUS_ERROR;
            if (ref->type != E_DATA && ref->extr == 0) return STATUS_ERROR;
            offset += strlen(ref->name);
            ret |= DST_DIR;
        }
    }

    return ret;
}