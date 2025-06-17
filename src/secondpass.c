#include "../include/secondpass.h"

uint32_t curr_address = 100;

int EncodeFile(char *input_path, char *output_path, char *ext_path, Label *labels, size_t *label_count, uint32_t icf, uint32_t dcf) {
    printf("(+) ENCODEFILE START: input='%s', output='%s', icf=%u, dcf=%u\n", input_path, output_path, icf, dcf);
    if (!input_path || !output_path || !labels || !label_count) return STATUS_ERROR;

    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) {
        printf("(-) Failed to open input file: %s\n", input_path);
        return STATUS_ERROR;
    }
    printf("(*) Opened input file: %s\n", input_path);

    FILE *output_fd = fopen(output_path, "w");
    if (!output_fd) {
        printf("(-) Failed to open output file: %s\n", input_path);
        fclose(input_fd);
        return STATUS_ERROR;
    }
    printf("(*) Opened output file: %s\n", output_path);

    FILE *extern_fd = fopen(ext_path, "w");
    if (!extern_fd) {
        printf("(-) Failed to open extern file: %s\n", ext_path);
        fclose(extern_fd);
        return STATUS_ERROR;
    }
    printf("(*) Opened extern file: %s\n", ext_path);

    fprintf(output_fd, "%u | %u\n", icf-100, dcf);
    printf("(*) Wrote header to output: %u | %u\n", icf, dcf);

    char line[MAX_LINE_LENGTH] = {0};
    while (fgets(line, MAX_LINE_LENGTH, input_fd) != NULL) {
        printf("(*) Processing line: %s", line);
        // Make a local copy of the line to avoid modifying the original
        char line_copy[MAX_LINE_LENGTH];
        strncpy(line_copy, line, MAX_LINE_LENGTH);
        line_copy[MAX_LINE_LENGTH - 1] = '\0';  // Ensure null termination

        char *comment_start = strchr(line_copy, COMMENT_DELIM);
        if (comment_start) *comment_start = '\0'; // Truncate comments

        // Skip label definition if needed
        char *ptr = strchr(line_copy, LABEL_DELIM);
        if (ptr) {
            ptr++;
        } else {
            ptr = line_copy; // no label
        }

        while (isspace(*ptr)) ptr++;

        // If is a DS directive, skip line
        if (strncmp(ptr, ISTRING, strlen(ISTRING)) == 0
        || strncmp(ptr, IDATA, strlen(IDATA)) == 0
        || strncmp(ptr, IENTRY, strlen(IENTRY)) == 0) {
            printf("(*) Skipping DS directive or meta line.\n");
            continue;
        }

        while (isspace(*ptr)) ptr++;

        // Handle instruction
        const Command *comm = FindCommand(ptr);
        if (!comm) {
            printf("(-) INVALID COMMAND NAME! -> %s\n", ptr);
            continue;
        }
        printf("(*) Found command: %s\n", comm->name);

        ptr += strlen(comm->name); 

        uint32_t word = 0;
        uint8_t add_modes = (uint8_t)DetermineAddressingModes(ptr, comm->opcount); 
        int non_reg = EncodeCommand(ptr, comm, add_modes, &word);
        printf("(*) Encoded command word:\n0b");
        for (int i = 31; i >= 0; i--) {
            printf("%d", (word >> i) & 1);
        }
        printf("\n0x%07X\n", word);
        // Emit first word
        fprintf(output_fd, "%07u : ", curr_address++);
        WordToHex(output_fd, word);
        printf("(*) Wrote command word at %u to output.\n", curr_address);

        // Tokenize the operands after command
        char *src = NULL, *dst = NULL;
        if (non_reg) {
            char op_copy[MAX_LINE_LENGTH];
            strncpy(op_copy, ptr, MAX_LINE_LENGTH);
            op_copy[MAX_LINE_LENGTH - 1] = '\0';
        
            src = strtok(op_copy, ",");
            dst = strtok(NULL, ",");
        }

        // Skip leading spaces
        if (src) while (isspace(*src)) src++;
        if (dst) while (isspace(*dst)) dst++;

        // Now emit additional words
        if (src) {
            if (*src == '#') {
                uint32_t imm = EncodeImm(src);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, imm);
                printf("(*) Encoded immediate src operand at %u: %s\n", curr_address, src);
            } else if (*src == '&') {
                uint32_t rel = EncodeRel(src + 1, labels, label_count, curr_address, extern_fd); // Skip '&'
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, rel);
                printf("(*) Encoded relative src operand at %u: %s\n", curr_address, src);
            } else if (*src != 'r') {
                uint32_t dir = EncodeDir(src, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, dir);
                printf("(*) Encoded direct src operand at %u: %s\n", curr_address, src);
            }
        }

        if (dst && (non_reg >= 1)) {
            if (*dst == '#') {
                uint32_t imm = EncodeImm(dst);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, imm);
                printf("(*) Encoded immediate dst operand: %s\n", dst);
            } else if (*dst == '&') {
                uint32_t rel = EncodeRel(dst + 1, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, rel);
                printf("(*) Encoded relative dst operand: %s\n", dst);
            } else if (*dst != 'r') {
                uint32_t dir = EncodeDir(dst, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, dir);
                printf("(*) Encoded direct dst operand: %s\n", dst);
            }
        }
    }

    fclose(input_fd);
    fclose(output_fd);
    printf("(+) ENCODEFILE END\n");
    return 0;
}