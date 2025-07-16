#include "../include/encoder.h"

int EncodeCommand(char *ops, const Command *comm, uint8_t modes, uint32_t *out) {
    assert(comm && out);

    int ret = comm->opcount;

    // LEGACY 24 BIT ENCODING //
    if (ASSEMBLER_FLAGS.legacy_24_bit) {
        *out |= (G_OP(comm->ident) << 18);              // Opcode
        *out |= ((G_FT(comm->ident)) << 3);             // Funct
        *out |= A;                                      // A bit
        

        // Operands are already verified
        if (comm->opcount == 2) {
            if ((modes & SRC_REG) == SRC_REG) {
                ret--;
                char *reg = strchr(ops, 'r');
                reg++;
                *out |= ((*reg - '0') << 13);           // src_reg
                *out |= (3 << 16);                      // src_add
            } else if ((modes & SRC_DIR) == SRC_DIR) {
                *out |= (1 << 16);                      // src_add
            } else if ((modes & SRC_REL) == SRC_REL) {
                *out |= (2 << 16);                      // src_add
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
            *out |= ((*reg - '0') << 8);                // dst_reg
            *out |= (3 << 11);                          // dst_add
        } else if ((modes & DST_DIR) == DST_DIR) {
            *out |= (1 << 11);                          // dst_add
        } else if ((modes & DST_REL) == DST_REL) {
            *out |= (2 << 11);                          // dst_add
        }
    }
    // CURRENT 32 BIT ENCODING //
    else {
        *out |= ((G_OP(comm->ident)) << 26);            // opcode
        *out |= ((G_FT(comm->ident)) << 4);             // Funct
        *out |= A;                                      // A bit

        if (comm->opcount == 2) {
            if ((modes & SRC_REG) == SRC_REG) {
                ret--;
                char *reg = strchr(ops, 'r');
                reg++;
                *out |= ((*reg - '0') << 18);           // src_reg
                *out |= (3 << 24);                      // src_add
            } else if ((modes & SRC_DIR) == SRC_DIR) {
                *out |= (1 << 24);                      // src_add
            } else if ((modes & SRC_REL) == SRC_REL) {
                *out |= (2 << 24);                      // src_add
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
            *out |= ((*reg - '0') << 10);               // dst_reg
            *out |= (3 << 16);                          // dst_add
        } else if ((modes & DST_DIR) == DST_DIR) {
            *out |= (1 << 16);                          // dst_add
        } else if ((modes & DST_REL) == DST_REL) {
            *out |= (2 << 16);                          // dst_add
        }

        if (ret > 0) *out |= M;                         // M bit
    }

    *out = WORD(*out);
    return ret;
}

uint32_t EncodeImm(char *op, bool is_last) {
    assert(op);
    
    // Skip spaces and '#'
    while (isspace(*op)) op++;
    op++;

    int32_t val = atoi(op);
    uint32_t ret = 0;

    if (ASSEMBLER_FLAGS.legacy_24_bit) {
        if (val < -(1<<20) || val > (1<<20) - 1) {
            printf("INVALID NUMBER: %s -> %d\n", op, val);
            return 0;
        }
        ret = ((val < 0) ? (uint32_t)(val + (1 << 21)) : (uint32_t)val) << 3;
    } else {
        if (val < -(1<<27) || val > (1<<27) - 1) {
            printf("INVALID NUMBER: %s -> %d\n", op, val);
            return 0;
        }
        ret = ((val < 0) ? (uint32_t)(val + (1 << 28)) : (uint32_t)val) << 4;
        if (!is_last) ret |= M;
    }

    ret |= A;
    return WORD(ret);
}

uint32_t EncodeDir(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, bool is_last) {
    assert(op && label_count);

    Label *label = FindLabel(op, labels, label_count);
    if (!label) { // Undefined label
        printf("UNDEFINED LABEL: %s\n", op);
        return 0;
    }

    uint32_t ret = 0;
    if (ASSEMBLER_FLAGS.legacy_24_bit) ret = ((uint32_t)(label->address)) << 3;
    else ret = ((uint32_t)(label->address)) << 4;
    if (label->extr > 0) {
        ret |= E;
    } else { 
        ret |= R;
    }

    // Check for extern
    if (label->extr > 0) {
        label->extr_used = true;
        if (label->use_count < MAX_EXTERN_USAGE) {
            label->used_at[label->use_count] = curr_address;
            label->use_count++;
            LogDebug("Updated usage for extern label %s. Count: %u", label->name, label->use_count);
        } else {
            printf("(*) Too many references to extern label!\n");
        }
        // fprintf(extern_fd, "%s: %08u\n", label->name, curr_address);
        // if (!ASSEMBLER_FLAGS.append_to_ext) ASSEMBLER_FLAGS.append_to_ext = true;
    }

    if (!ASSEMBLER_FLAGS.legacy_24_bit && !is_last) ret |= M;
    return WORD(ret);
}

uint32_t EncodeRel(char *op, Label labels[MAX_LABELS], size_t *label_count, uint32_t curr_address, bool is_last) {
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

    uint32_t ret = 0;
    if (ASSEMBLER_FLAGS.legacy_24_bit) ret = ((val < 0) ? (uint32_t)(val + (1 << 21)) : (uint32_t)val) << 3;
    else ret = ((val < 0) ? (uint32_t)(val + (1 << 28)) : (uint32_t)val) << 4;
    ret |= A;

    // Check for extern
    if (label->extr > 0) {
        label->extr_used = true;
        if (label->use_count < MAX_EXTERN_USAGE) {
            label->used_at[label->use_count] = curr_address;
            label->use_count++;
            LogDebug("Updated usage for extern label %s. Count: %u", label->name, label->use_count);
        } else {
            printf("(*) Too many references to extern label!\n");
        }
        // fprintf(extern_fd, "%s: %08u\n", label->name, curr_address);
        // if (!ASSEMBLER_FLAGS.append_to_ext) ASSEMBLER_FLAGS.append_to_ext = true;
    }

    if (!ASSEMBLER_FLAGS.legacy_24_bit && !is_last) ret |= M;
    return WORD(ret);
}

void WordToHex(FILE *file, uint32_t num) {
    num = WORD(num);
    if (ASSEMBLER_FLAGS.legacy_24_bit) fprintf(file, "0x%06X\n", num);
    else fprintf(file, "0x%08X\n", num);
}