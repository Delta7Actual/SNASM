#include "../include/secondpass.h"

uint32_t curr_address = 100;

int EncodeFile(char *input_path, char *output_path, char *extern_path, char *entry_path, Label *labels, size_t *label_count, uint32_t icf, uint32_t dcf) {
    if (!input_path || !output_path || !labels || !label_count) return STATUS_ERROR;

    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) {
        printf("(-) Failed to open input file: %s\n", input_path);
        return STATUS_ERROR;
    }

    FILE *output_fd = NULL;
    if (ASSEMBLER_FLAGS.append_to_out) output_fd = fopen(output_path, "a");
    else output_fd = fopen(output_path, "w");
    if (!output_fd) {
        printf("(-) Failed to open output file: %s\n", input_path);
        fclose(input_fd);
        return STATUS_ERROR;
    }

    FILE *extern_fd = NULL;
    if (ASSEMBLER_FLAGS.gen_externals) {
        if (ASSEMBLER_FLAGS.append_to_ext) extern_fd = fopen(extern_path, "a");
        else extern_fd = fopen(extern_path, "w");
        if (!extern_fd) {
            printf("(-) Failed to open extern file: %s\n", extern_path);
            fclose(input_fd);
            fclose(output_fd);
            return STATUS_ERROR;
        }
    }

    FILE *entry_fd = NULL;
    if (ASSEMBLER_FLAGS.gen_entries) {
        if (ASSEMBLER_FLAGS.append_to_ent) entry_fd = fopen(entry_path, "a");
        else entry_fd = fopen(entry_path, "w");
        if (!entry_fd) {
            printf("(-) Failed to open entry file: %s\n", entry_path);
            fclose(input_fd);
            fclose(output_fd);
            if (extern_fd) fclose(extern_fd);
            return STATUS_ERROR;
        }
    }

    if (!ASSEMBLER_FLAGS.append_to_out) {
        fprintf(output_fd, "%u | %u\n", icf-100, dcf);
        LogDebug("Wrote header to output: %u | %u\n", icf, dcf);
    }

    // Data segment
    uint32_t *data_segment = calloc(dcf+1, sizeof(uint32_t));
    if (!data_segment) {
        printf("(-) Error: failed to allocate data segment!");
        fclose(input_fd);
        fclose(output_fd);
        fclose(extern_fd);
        fclose(entry_fd);
        return STATUS_ERROR;
    }

    data_segment[0] = 1; // Start from idx = 1

    char line[MAX_LINE_LENGTH] = {0};
    while (fgets(line, MAX_LINE_LENGTH, input_fd) != NULL) {
        TrimNewline(line);
        LogDebug("Processing line: %s\n", line);
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

        // If is a .extern or directive, continue
        if (strncmp(ptr, IEXTERN, strlen(IEXTERN)) == 0) {
            LogDebug("Skipping '.extern' directive!\n");
            continue;
        }

        // If is a .entry directive, add to .ent
        if (strncmp(ptr, IENTRY, strlen(IENTRY)) == 0) {
            if (ASSEMBLER_FLAGS.gen_entries) {
                ptr += strlen(IENTRY);
                while (isspace(*ptr)) ptr++;
                
                Label *found = FindLabel(ptr, labels, label_count);
                if (!found) {
                    printf("(-) Error: Found .entry directive but couldn't find label definition! <-- %s\n", ptr);
                    continue;
                }

                ASSEMBLER_FLAGS.append_to_ent = true;
                fprintf(entry_fd, "%s: %07zu\n", found->name, found->address);
                LogDebug("Wrote entry directive to entries file!\n");
            }
        continue;
        }

        // If it is a DS directive, add to data section
        if (strncmp(ptr, ISTRING, strlen(ISTRING)) == 0
        || strncmp(ptr, IDATA, strlen(IDATA)) == 0) {
            HandleDSDirective(ptr, data_segment);
        }

        while (isspace(*ptr)) ptr++;

        // Handle instruction
        char mnemonic[MAX_LINE_LENGTH] = {0};
        sscanf(ptr, "%s", mnemonic);

        const Command *comm = FindCommand(mnemonic);
        if (!comm) {
            LogDebug("Skipping non-command line %s\n", ptr);
            continue;
        }
        LogDebug("Found command: %s\n", comm->name);

        ptr += strlen(comm->name); 

        uint32_t word = 0;
        uint8_t add_modes = (uint8_t)DetermineAddressingModes(ptr, comm->opcount); 
        int non_reg = EncodeCommand(ptr, comm, add_modes, &word);
        LogDebug("Encoded command word:\n");
        if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
            LogDebug("Hex: 0x%07X | Bin: 0b", word);
            LogU32AsBin(word);
        }

        // Emit first word
        fprintf(output_fd, "%07u : ", curr_address++);
        WordToHex(output_fd, word);
        LogDebug("Wrote command word at %u to output.\n", curr_address);

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

                LogDebug("Encoded immediate operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", imm);
                    LogU32AsBin(imm);
                }
            } else if (*src == '&') {
                uint32_t rel = EncodeRel(src + 1, labels, label_count, curr_address, extern_fd); // Skip '&'
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, rel);

                LogDebug("Encoded relative operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", rel);
                    LogU32AsBin(rel);
                }
            } else if (*src != 'r') {
                uint32_t dir = EncodeDir(src, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, dir);

                LogDebug("Encoded direct operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", dir);
                    LogU32AsBin(dir);
                }
            }
        }

        if (dst && (non_reg >= 1)) {
            if (*dst == '#') {
                uint32_t imm = EncodeImm(dst);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, imm);
                
                LogDebug("Encoded immediate operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", imm);
                    LogU32AsBin(imm);
                }
            } else if (*dst == '&') {
                uint32_t rel = EncodeRel(dst + 1, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, rel);

                LogDebug("Encoded relative operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", rel);
                    LogU32AsBin(rel);
                }
            } else if (*dst != 'r') {
                uint32_t dir = EncodeDir(dst, labels, label_count, curr_address, extern_fd);
                fprintf(output_fd, "%07u : ", curr_address++);
                WordToHex(output_fd, dir);

                LogDebug("Encoded direct operand at %u:\n", curr_address-1);
                if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
                    LogDebug("Hex: 0x%06X | Bin: 0b", dir);
                    LogU32AsBin(dir);
                }
            }
        }
    }

    for (uint32_t i = 1; i < data_segment[0]; i++) {
        fprintf(output_fd, "%07u : ", curr_address++);
        WordToHex(output_fd, data_segment[i]);
        LogDebug("Wrote to data segment at %u!\n", curr_address-1);
    }

    LogVerbose("Successfully encoded %s. Wrote %u words to output\n", input_path, icf + dcf - 101);
    LogVerbose("Text-Section begins at %u, ends at %u\n", 100, icf -1);
    if (data_segment[0] > 1)
        LogVerbose("Data-Segment begins at %u, ends at %u\n", icf, icf + dcf - 1);

    // Set append flag
    if (!ASSEMBLER_FLAGS.append_to_out) ASSEMBLER_FLAGS.append_to_out = true;

    // Cleanup
    fclose(input_fd);
    fclose(output_fd);
    if (extern_fd) fclose(extern_fd);
    if (entry_fd)  fclose(entry_fd);

    free(data_segment);
    
    return 0;
}