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

    FILE *file_fd = fopen(file_path, "r");
    if (!file_fd) return STATUS_ERROR;

    char line[MAX_LINE_LENGTH] = {0};
    char *entries[MAX_LABELS] = {0};
    char *externals[MAX_LABELS] = {0};
    size_t entry_count = 0;
    size_t extern_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {

        LogDebug("Curr: IC->%d/DC->%d\n", IC, DC);

        // Skip leading spaces manually
        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') continue; // Empty or whitespace-only line

        // Handle .entry and .extern directives
        if (strncmp(ptr, IENTRY, strlen(IENTRY)) == 0) {
            ptr += strlen(IENTRY);
            while (isspace((unsigned char)*ptr)) ptr++;  // Skip spaces

            if (*ptr == '\0') {
                LogInfo("Error: Missing label in .entry directive\n");
                continue;
            }

            char *entry_label = TrimWhitespace(ptr);
            Label *existing = FindLabel(entry_label, labels, label_count);
            if (existing) {
                int isExternInFile = 0;
                for (size_t i = 0; i < extern_count; i++) {
                    if (strncmp(existing->name, externals[i], strlen(existing->name)) == 0) {
                        printf("(-) Label cannot be defined as both extern and entry in the same file! -> %s"
                            , existing->name);
                        isExternInFile = 1;
                        break;
                    }
                } 
                
                if (!isExternInFile) existing->entr = 1;
            } else {
                entries[entry_count] = strdup(entry_label);
                entry_count++;
                LogDebug("Parsed entry directive\n");
            }
            continue;
        }

        if (strncmp(ptr, IEXTERN, strlen(IEXTERN)) == 0) {
            ptr += strlen(IEXTERN);
            while (isspace((unsigned char)*ptr)) ptr++;  // Skip spaces

            if (*ptr == '\0') {
                printf("Error: Missing label in .extern directive\n");
                continue;
            }

            char *extern_label = TrimWhitespace(ptr);
            Label *existing = FindLabel(extern_label, labels, label_count);
            if (existing) {
                printf("Error: Label %s was already defined, but extern was declared!\n", extern_label);
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

        int status = AddLabel(line, curr);
        if (status == STATUS_NO_RESULT) {
            // No label, either DS directives or instructions
            free(curr);
            
            // Handle `.data` and `.string` directives
            if (strncmp(line, ISTRING, strlen(ISTRING)) == 0
            || strncmp(line, IDATA, strlen(IDATA)) == 0) {
                int values = HandleDSDirective(line, NULL);
                if (values < 0) {
                    printf("Error in size calculation in line: %s\n", line);
                    continue;
                }
                DC += values;
            }
            else { // Instruction or comment
                int offset = 0;
                while(isspace(line[offset])) offset++;
                if (line[offset] == COMMENT_DELIM) continue;

                const Command *com = FindCommand(line);
                if (!com) {
                    printf("Error parsing instruction in line: %s\n", line);
                    continue;
                }

                int words = ValidateCommand(line, com);
                if (words < 0) {
                    printf("Error in size calculation in line: %s\n", line);
                    continue;
                }
                IC += words;
            }
            // Continue to next line
            continue;
        } else if (status == STATUS_ERROR) {
            free(curr);
            printf("ERROR IN LINE: %s\n", line);
            continue;
        }

        // Locate the colon (`:`) manually
        char *rest = strchr(ptr, LABEL_DELIM);
        if (!rest) {
            printf("Error, malformed label in line: %s\n", line);
            free(curr);
            return STATUS_ERROR;
        }
        rest++;  // Move past the colon

        while (isspace((unsigned char)*rest)) rest++;  // Skip spaces after colon

        if (FindLabel(curr->name, labels, label_count) != NULL) {
            printf("Multiple definitions of label: %s!\n", curr->name);
            free(curr);
            continue;
        }

        if (curr->type == E_DATA) {
            curr->address = DC;
            int values =  HandleDSDirective(rest, NULL);
            if (values < 0) printf("Error calculating data size for label %s!, %s\n", curr->name, rest);
            else DC += values;
        } else {
            curr->address = IC;
            const Command *com = FindCommand(rest);
            if (com) {
                int words = ValidateCommand(rest, com);
                if (words > 0) {
                    IC += words;
                } else {
                    printf("Illegal command parameters in label: %s: %s\n", curr->name, rest);
                }
            } else {
                printf("Illegal command in label: %s!, %s\n", curr->name, rest);
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
            printf("Error: .entry label %s is not defined in this file!\n", entries[i]);
        } else {
            entry->entr = 1;
        }

        free(entries[i]);
    }

    LogVerbose("Generated symbol table for file %s.\n", file_path);
    LogVerbose("Found %zu entry point(s) and %zu external reference(s)\n", entry_count, extern_count);
    LogVerbose("Compiled %zu symbols in file %s\n", *label_count, file_path);

    fclose(file_fd);
    return 0;
}

int ValidateSymbolTable(Label labels[MAX_LABELS], size_t *label_count) {
    if (!labels || !label_count) return STATUS_ERROR;

    int status = 0;
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].entr != labels[i].extr) {
            status++;
            LogDebug("Warning: Failed to find matching %s for label %s in program!\n", 
            (labels[i].entr == 0) ? ".entry" : ".extern", labels[i].name);
        }
        if (labels[i].type == E_DATA) labels[i].address += ICF;
    }

    return status;
}