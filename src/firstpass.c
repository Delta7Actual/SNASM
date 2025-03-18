#include "../include/firstpass.h"

int HandleDataLabel(char *token) {
    if (!token) return STATUS_ERROR;

    if (strncmp(token, IDATA, strlen(IDATA)) == 0) {
        if (*token == POS_DELIM || *token == NEG_DELIM) token++;
        
        int values = 1;
        while (token) {
            if (!isalnum(*token) && *token != ',') return STATUS_ERROR;

            char *token = strtok(token, ",");
            while (*token) {
                values++;
                token = strtok(NULL, ",");
            }
        }
        return values;
    }
    if (strncmp(token, ISTRING, strlen(ISTRING)) == 0) {
        if (*token != '\"') return STATUS_ERROR;
        token++;

        int values = 0;
        while (isalpha(*token) && *token != '\"') {
            values++;
            token++;
        }

        return values;
    }

    return STATUS_ERROR;
}

int BuildSymbolTable(char *file_path, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_path || !labels || !label_count) return STATUS_ERROR;

    FILE *file_fd = fopen(file_path, "r");
    if (!file_fd) return STATUS_ERROR;

    char line[MAX_LINE_LENGTH] = {0};
    char *entries[MAX_LABELS]  = {0};
    size_t entry_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        char *trimmed = TrimWhitespace(line);
        if (!trimmed || *trimmed == '\0') continue; // Skip empty lines

        // Handle instructions


        // Handle .entry and .extern
        if (strncmp(trimmed, IENTRY, strlen(IENTRY)) == 0) {
            char *entry_label = TrimWhitespace(trimmed + strlen(IENTRY));
            if (!entry_label || *entry_label == '\0') {
                printf("Error: Missing label in .entry directive\n");
                continue;
            }

            Label *existing = FindLabel(entry_label, labels, label_count);
            if (existing) {
                existing->entr = 1;
            } else {
                printf("Warning: .entry label %s is not defined in this file (will check in second pass)\n", entry_label);
                entries[entry_count] = strndup(entry_label, strlen(entry_label));
                entry_count++;
            }
            continue;
        }

        if (strncmp(trimmed, IEXTERN, strlen(IEXTERN)) == 0) {
            char *extern_label = TrimWhitespace(trimmed + strlen(IEXTERN));
            if (!extern_label || *extern_label == '\0') {
                printf("Error: Missing label in .extern directive\n");
                continue;
            }

            // Check if the label already exists
            Label *existing = FindLabel(extern_label, labels, label_count);
            if (existing) {
                printf("Error: Label %s was already defined, but extern was declared!\n", extern_label);
                continue;
            }

            // Store the label as EXTERN
            labels[*label_count].name = strndup(extern_label, strlen(extern_label));
            labels[*label_count].address = 0;
            labels[*label_count].extr = 1;
            (*label_count)++;

            continue;
        }

        // Handle Label Definitions
        Label *curr = malloc(sizeof(Label));
        if (!curr) {
            fclose(file_fd);
            return STATUS_ERROR;
        }

        int status = AddLabel(line, curr);
        if (status == STATUS_NO_RESULT) {
            free(curr);
            continue;
        } else if (status == STATUS_ERROR) {
            free(curr);
            printf("ERROR IN LINE: %s\n", line);
            continue;
        }

        // Process label if found
        char *rest = strchr(trimmed, ':');
        if (!rest) {
            free(curr);
            return STATUS_ERROR;
        }
        rest++;

        if (FindLabel(curr->name, labels, label_count) != NULL) {
            printf("Multiple definitions of label: %s!\n", curr->name);
            free(curr);
            continue;
        }

        if (curr->type == E_DATA) {
            curr->address = DC;
            DC += HandleDataLabel(rest); // Forward Data Counter
        }
        else {
            curr->address = IC;
            const Command *com = FindCommand(rest);
            if (com) {
                int words = ValidateCommand(rest, com, labels, label_count);
                if (words > 0) {
                    IC += words; // Forward Instruction Counter
                } else {
                    printf("Illegal command parameters in label: %s!, %s\n", curr->name, rest);
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
    for (size_t i = 0; i < entry_count; i++) {
        Label *entry = FindLabel(entries[i], labels, label_count);
        if (!entry) {
            printf("Error: .entry label %s is not defined in this file!\n", entries[i]);
        } else {
            entry->entr = 1; // Mark the label as an entry
        }

        free(entries[i]);
    }

    fclose(file_fd);
    return 0;
}

// int FormatExtEntFiles(char *file_name, Label labels[MAX_LABELS], size_t *label_count) {

// }