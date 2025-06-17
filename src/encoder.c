#include "../include/encoder.h"

int EncodeCommand(char *ops, const Command *comm, uint8_t modes, uint32_t *out) {
    assert(comm && out);

    printf("(*) Encoding command first word: %s, ops: %s\n", comm->name, ops);

    printf("-> ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (*out >> i) & 1);
    }
    printf("\n");
    *out |= (G_OP(comm->ident) << 18);
    printf("-> ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (*out >> i) & 1);
    }
    printf("\n");
    *out |= ((G_FT(comm->ident)) << 3);
    printf("-> ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (*out >> i) & 1);
    }
    printf("\n");
    *out |= A;
    printf("-> ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (*out >> i) & 1);
    }
    printf("\n");
    
    int ret = comm->opcount;

    // Operands are already verified
    if (comm->opcount == 2) {
        if ((modes & SRC_REG) == SRC_REG) {
            ret--;
            char *reg = strchr(ops, 'r');
            reg++;
            *out |= ((*reg - '0') << 13);
            *out |= (3 << 16);
        } else if ((modes & SRC_DIR) == SRC_DIR) {
            *out |= (1 << 16);
        } else if ((modes & SRC_REL) == SRC_REL) {
            *out |= (2 << 16);
        }
    }

    if ((modes & DST_REG) == DST_REG) {
        ret--;
        char *comma = strchr(ops, ',');
        if (comma) {
            comma++;
        } else {
            comma = ops;
        }
        char *reg = strchr(comma, 'r');
        reg++;
        *out |= ((*reg - '0') << 8);
        *out |= (3 << 11);
    } else if ((modes & DST_DIR) == DST_DIR) {
        *out |= (1 << 11);
    } else if ((modes & DST_REL) == DST_REL) {
        *out |= (2 << 11);
    }

    *out = WORD(*out); // Mask to 24 bits
    return ret;
}

uint32_t EncodeImm(char *op) {
    assert(op);
    
    // Skip spaces and '#'
    while (isspace(*op)) op++;
    op++;

    int32_t val = atoi(op);
    if (val < -(1<<20) || val > (1<<20)) {
        printf("INVALID NUMBER: %s -> %d\n", op, val);
        return 0;
    }

    uint32_t ret = ((val < 0) ? (uint32_t)(val + (1 << 21)) : (uint32_t)val) << 3;
    ret |= A;

    printf("IMM: val: %s -> %d --> %06X | ", op, val, ret);
    WordToBin(ret);

    return WORD(ret);
}

uint32_t EncodeDir(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, FILE *extern_fd) {
    assert(op && label_count);

    Label *label = FindLabel(op, labels, label_count);
    if (!label) { // Undefined label
        printf("UNDEFINED LABEL: %s\n", op);
        return 0;
    }

    uint32_t ret = ((uint32_t)(label->address)) << 3;
    if (label->extr > 0) {
        ret |= E;
    } else { 
        ret |= R;
    }

    // Check for extern
    if (label->extr > 0) fprintf(extern_fd, "%s: %07u\n", label->name, curr_address);

    return WORD(ret);
}

uint32_t EncodeRel(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, FILE *extern_fd) {
    assert(op && label_count);

    Label *label = FindLabel(op, labels, label_count);
    if (!label) { // Undefined label
        printf("UNDEFINED LABEL: %s\n", op);
        return 0;
    }

    int32_t val = label->address + 1 - curr_address;

    if (val < -(1<<20) || val > (1<<20)) {
        printf("INVALID NUMBER: %s -> %d\n", op, val);
        return 0;
    }

    uint32_t ret = ((val < 0) ? (uint32_t)(val + (1 << 21)) : (uint32_t)val) << 3;
    ret |= A;

    // Check for extern
    if (label->extr > 0) fprintf(extern_fd, "%s: %07u\n", label->name, curr_address);

    return ret;
}

void WordToHex(FILE *file, uint32_t num) {
    num = WORD(num);
    fprintf(file, "0x%06X\n", num);
}

void WordToBin(uint32_t num) {
    num = WORD(num);
    for (int i = 23; i >= 0; i--) {
        putchar((num & (1 << i)) ? '1' : '0');
        if (i % 4 == 0 && i != 0) putchar(' '); // Group by 4 bits
    }
    putchar('\n');
}