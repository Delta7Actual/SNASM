#include "../include/firstpass.h"
#include "../include/logger.h"

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
    char *entries[MAX_LABELS] = {0};
    size_t entry_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        // Skip leading spaces manually
        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') continue; // Empty or whitespace-only line

        // Handle .entry and .extern directives
        if (strncmp(ptr, IENTRY, strlen(IENTRY)) == 0) {
            ptr += strlen(IENTRY);
            while (isspace((unsigned char)*ptr)) ptr++;  // Skip spaces

            if (*ptr == '\0') {
                printf("Error: Missing label in .entry directive\n");
                continue;
            }

            Label *existing = FindLabel(ptr, labels, label_count);
            if (existing) {
                existing->entr = 1;
            } else {
                printf("Warning: .entry label %s is not defined in this file (will check in second pass)\n", ptr);
                entries[entry_count] = strndup(ptr, strlen(ptr));
                entry_count++;
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

            Label *existing = FindLabel(ptr, labels, label_count);
            if (existing) {
                printf("Error: Label %s was already defined, but extern was declared!\n", ptr);
                continue;
            }

            labels[*label_count].name = strndup(ptr, strlen(ptr));
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

        // Locate the colon (`:`) manually
        char *rest = strchr(ptr, LABEL_DELIM);
        if (!rest) {
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
            DC += HandleDataLabel(rest); // Forward Data Counter
        } else {
            curr->address = IC;
            const Command *com = FindCommand(rest);
            if (com) {
                int words = ValidateCommand(rest, com, labels, label_count);
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
    for (size_t i = 0; i < entry_count; i++) {
        Label *entry = FindLabel(entries[i], labels, label_count);
        if (!entry) {
            printf("Error: .entry label %s is not defined in this file!\n", entries[i]);
        } else {
            entry->entr = 1;
        }

        free(entries[i]);
    }

    fclose(file_fd);
    return 0;
}

/* Generates the `.ent` file */
int FormatEntryFile(char *file_name, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_name || !labels || !label_count) {
        LogError(CONTEXT, "FormatEntryFile", "Received NULL input");
        return STATUS_ERROR;
    }

    char output_name[MAX_FILENAME_LENGTH] = {0};
    snprintf(output_name, MAX_FILENAME_LENGTH, "%s%s", file_name, ENTRIES_FILE_EXTENSION);

    FILE *output_fd = fopen(output_name, "w");
    if (!output_fd) {
        LogError(CONTEXT, "FormatEntryFile", "Failed to open .ent file");
        return -1;
    }

    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].entr) {
            fprintf(output_fd, "%s: %07d\n", labels[i].name, labels[i].address);
        }
    }

    fclose(output_fd);
    return 0;
}