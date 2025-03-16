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

// We shouldnt need to pass the symbol table here, should be a better way
// TODO: Change this
int ValidateCommand(char *com_line, const Command *comm, Symbol symbols[MAX_LABELS], size_t *symbol_count) {
    if (!com_line || !comm) return STATUS_CATASTROPHIC;

    int offset = 0;
    while (isspace(com_line[offset])) offset++; 
    if (strncmp(com_line, comm->name, strlen(comm->name)) != 0) return STATUS_CATASTROPHIC;
    
    while (isspace(com_line[offset])) offset++;

    uint8_t modes = DetermineAddressingModes(com_line, symbols, symbol_count);
    if (comm->addmodes & modes != modes) return STATUS_CATASTROPHIC; // Missmatched addressing
    
    int words = 1;
    while (modes > 0) {
        modes &= (modes - 1);
        words++;
    }
    if (comm->opcount != words - 1) return STATUS_CATASTROPHIC; // Wrong opcount
    
    return words; // 1 word for command + 1 for each operand
}

// Returns -1 if syntax error
uint8_t DetermineAddressingModes(char *operand, Symbol symbols[MAX_LABELS], size_t *symbol_count) {
    if (!operand || !symbols || !symbol_count) return STATUS_CATASTROPHIC;

    uint8_t ret = 0;
    int offset = 0;

    // Immediate
    if (operand[0] == '#') {
        offset++;
        while (operand[offset]) {
            if (!isdigit(operand[offset]) && operand[offset] != ',') return STATUS_CATASTROPHIC;
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
        Symbol *ref = FindSymbol(operand, symbols, symbol_count);
        if (!ref) return STATUS_CATASTROPHIC;
        if (ref->extr > 0) return STATUS_CATASTROPHIC;
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
        Symbol *ref = FindSymbol(operand[offset], symbols, symbol_count);
        if (!ref) return STATUS_CATASTROPHIC;
        if (ref->type != E_DATA && ref->extr == 0) return STATUS_CATASTROPHIC;
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
                if (!isdigit(operand[offset]) && operand[offset] != ',') return STATUS_CATASTROPHIC;
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
            Symbol *ref = FindSymbol(operand, symbols, symbol_count);
            if (!ref) return STATUS_CATASTROPHIC;
            if (ref->extr > 0) return STATUS_CATASTROPHIC;
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
            Symbol *ref = FindSymbol(operand[offset], symbols, symbol_count);
            if (!ref) return STATUS_CATASTROPHIC;
            if (ref->type != E_DATA && ref->extr == 0) return STATUS_CATASTROPHIC;
            offset += strlen(ref->name);
            ret |= DST_DIR;
        }
    }

    return ret;
}