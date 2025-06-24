#include "../include/firstpass.h"
#include "../include/logger.h"
#include "../include/parser.h" // Ensure TrimWhitespace is available

#include "../include/firstpass.h"

uint32_t IC  = 0;
uint32_t DC  = 0;
uint32_t ICF = 0;
uint32_t DCF = 0;

int BuildSymbolTable(char *file_path, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_path || !labels || !label_count) return STATUS_ERROR;

    int status = 0;

    FILE *file_fd = fopen(file_path, "r");
    if (!file_fd) return STATUS_ERROR;

    char line[MAX_LINE_LENGTH] = {0};
    char *entries[MAX_LABELS] = {0};
    char *externals[MAX_LABELS] = {0};
    size_t entry_count = 0;
    size_t extern_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        LogDebug("Curr: IC->%d/DC->%d\n", IC, DC);

        // Remove comment first
        char *comment = strchr(line, COMMENT_DELIM);
        if (comment) *comment = '\0';

        // Now trim whitespace on the cleaned line
        char *ptr = TrimWhitespace(line);
        if (*ptr == '\0') continue; // Line is empty or only spaces/comments

        // Handle .entry and .extern directives
        if (strncmp(ptr, IENTRY, strlen(IENTRY)) == 0) {
            ptr += strlen(IENTRY);
            while (isspace((unsigned char)*ptr)) ptr++;  // Skip spaces

            if (*ptr == '\0') {
                LogInfo("Error: Missing label in .entry directive\n");
                status = STATUS_ERROR;
                continue;
            }

            char *entry_label = TrimWhitespace(ptr);
            Label *existing = FindLabel(entry_label, labels, label_count);
            if (existing) {
                int isExternInFile = 0;
                for (size_t i = 0; i < extern_count; i++) {
                    if (strncmp(existing->name, externals[i], strlen(existing->name)) == 0) {
                        printf("(-) Label %s cannot be defined as both extern and entry in the same file!\n"
                            , existing->name);
                        isExternInFile = 1;
                        break;
                    }
                } 
                
                if (!isExternInFile) {
                    entries[entry_count] = strdup(existing->name);
                    entry_count++;
                    LogDebug("Parsed entry directive\n");
                    ASSEMBLER_FLAGS.entry_point_exists = true;
                }
            } else {
                entries[entry_count] = strdup(entry_label);
                entry_count++;
                LogDebug("Parsed entry directive\n");
                ASSEMBLER_FLAGS.entry_point_exists = true;
            }
            continue;
        }

        if (strncmp(ptr, IEXTERN, strlen(IEXTERN)) == 0) {
            ptr += strlen(IEXTERN);
            while (isspace((unsigned char)*ptr)) ptr++;  // Skip spaces

            if (*ptr == '\0') {
                printf("Error: Missing label in .extern directive\n");
                status = STATUS_ERROR;
                continue;
            }

            char *extern_label = TrimWhitespace(ptr);
            Label *existing = FindLabel(extern_label, labels, label_count);
            if (existing) {
                // If label is already marked as extern, that's fine
                if (!existing->extr) {
                    // Defined in this same file
                    existing->extr = 1;
                    LogDebug("Warning: %s declared extern but already defined; assuming multi-file linking.\n", extern_label);                    continue;
                }
                continue;
            }

            memset(&labels[*label_count], 0, sizeof(Label));
            labels[*label_count].name = strdup(extern_label);
            labels[*label_count].address = 0;
            labels[*label_count].extr = 1;
            (*label_count)++;

            externals[extern_count] = strdup(extern_label);
            extern_count++;

            LogDebug("Parsed extern directive\n");

            continue;
        }

        // Handle Label Definitions
        Label *curr = malloc(sizeof(Label));
        if (!curr) {
            fclose(file_fd);
            return STATUS_ERROR;
        }
        memset(curr, 0, sizeof(Label));

        int label_status = AddLabel(ptr, curr);
        if (label_status == STATUS_NO_RESULT) {
            // No label, either DS directives or instructions
            free(curr);
            
            // Handle `.data` and `.string` directives
            if (strncmp(ptr, ISTRING, strlen(ISTRING)) == 0
            || strncmp(ptr, IDATA, strlen(IDATA)) == 0) {
                int values = HandleDSDirective(ptr, NULL);
                if (values < 0) {
                    printf("Error in size calculation in line: %s", line);
                    status = STATUS_ERROR;
                    continue;
                }
                DC += values;
            }
            else { // Instruction or comment
                int offset = 0;
                while(isblank(ptr[offset])) offset++;
                if (ptr[offset] == COMMENT_DELIM) continue;

                char *clean_line = ptr + offset;
                char mnemonic[MAX_LINE_LENGTH] = {0};

                sscanf(clean_line, "%s", mnemonic);
                const Command *com = FindCommand(mnemonic);
                if (!com) {
                    printf("(-) Error: parsing instruction in line: %s\n", line);
                    status = STATUS_ERROR;
                    continue;
                }

                int words = ValidateCommand(ptr + offset, com);
                if (words < 0) {
                    printf("(-) Error in size calculation in line: %s\n", line);
                    status = STATUS_ERROR;
                    continue;
                }
                IC += words;
            }
            // Continue to next line
            continue;
        } else if (label_status == STATUS_ERROR) {
            free(curr);
            printf("(-) Error: AddLabel failed with status:{-1} in line: %s", line);
            status = STATUS_ERROR;
            continue;
        }

        // Locate the colon (`:`) manually
        char *rest = strchr(ptr, LABEL_DELIM);
        if (!rest) {
            printf("(-) Error: malformed label in line: %s\n", line);
            free(curr);
            status = STATUS_ERROR;
            continue;
        }
        rest++;  // Move past the colon

        while (isspace((unsigned char)*rest)) rest++;  // Skip spaces after colon

        Label *found = FindLabel(curr->name, labels, label_count);
        if (found) {
            if (found->extr) {
                // Was previously extern
                found->address = (curr->type == E_DATA) ? DC : IC;
                found->type = curr->type;
                found->extr = 0; // No longer external
                LogDebug("Updated previously extern label %s to local definition\n", curr->name);
                // Adjust counters
                if (curr->type == E_DATA) {
                    int values = HandleDSDirective(rest, NULL);
                    if (values >= 0) DC += values;
                } else {
                    const Command *com = FindCommand(rest);
                    if (com) {
                        int words = ValidateCommand(rest, com);
                        if (words > 0) IC += words;
                    }
                }
                free(curr);
                continue;
        } else {
            // Fully defined already
            printf("(-) Error: Multiple definitions of label: %s!\n", curr->name);
            free(curr);
            status = STATUS_ERROR;
            continue;
        }
    }

        if (curr->type == E_DATA) {
            curr->address = DC;
            int values =  HandleDSDirective(rest, NULL);
            if (values < 0) {
                printf("(-) Error: Failed to calculate data size for label %s!, %s\n", curr->name, rest);
                status = STATUS_ERROR;
            }
            else DC += values;
        } else {
            curr->address = IC;
            
            char mnemonic[MAX_LINE_LENGTH] = {0};
            sscanf(rest, "%s", mnemonic);

            const Command *com = FindCommand(mnemonic);
            if (com) {
                int words = ValidateCommand(rest, com);
                if (words > 0) {
                    IC += words;
                } else {
                    printf("(-) Error: Illegal command parameters in label: %s: %s\n", curr->name, rest);
                    status = STATUS_ERROR;
                }
            } else {
                printf("(-) Error: Illegal command in label: %s!, %s\n", curr->name, rest);
                status = STATUS_ERROR;
            }
        }

        // Store label
        labels[*label_count] = *curr;
        (*label_count)++;

        free(curr);
    }

    // Re-check entries
    LogDebug("Validating entry definitions...\n");
    for (size_t i = 0; i < entry_count; i++) {
        Label *entry = FindLabel(entries[i], labels, label_count);
        if (!entry) {
            printf("(-) Error: .entry label %s is not defined in this file!\n", entries[i]);
            status = STATUS_ERROR;
        } else {
            entry->entr = 1;
        }

        free(entries[i]);
    }

    if (status == 0) {
        LogVerbose("Generated symbol table for file %s.\n", file_path);
        LogVerbose("Found %zu entry point(s) and %zu external reference(s)\n", entry_count, extern_count);
        LogVerbose("Compiled %zu symbols in file %s\n", *label_count, file_path);
    }

    fclose(file_fd);
    return status;
}

int ValidateSymbolTable(Label labels[MAX_LABELS], size_t *label_count) {
    if (!labels || !label_count) return STATUS_ERROR;

    int status = 0;
    bool is_start = false;
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].entr && strcmp(labels[i].name, "START") == 0) {
            LogDebug("Found entry point START!\n");
            is_start = true;
            ASSEMBLER_FLAGS.start_exists = true;
        }
        if (labels[i].entr != labels[i].extr) {
            status++;
            LogDebug("Warning: Failed to find matching %s for label %s in program!\n", 
            (labels[i].entr == 0) ? ".entry" : ".extern", labels[i].name);
        }
        if (labels[i].type == E_DATA) labels[i].address += ICF;
    }

    if (!is_start) {
        LogInfo("(*) Warning: Could not find START in program!\n");
    }

    return status;
}