#include "../include/firstpass.h"
#include "../include/logger.h"

static char CONTEXT[] = "FirstPass";

/* Parses a `.data` or `.string` directive and returns the number of memory words needed */
int HandleDataLabel(char *token) {
    if (!token) {
        LogError(CONTEXT, "HandleDataLabel", "Received NULL token");
        return STATUS_ERROR;
    }

    if (strncmp(token, IDATA, strlen(IDATA)) == 0) {
        if (*token == POS_DELIM || *token == NEG_DELIM) token++;

        int values = 1;
        while (token) {
            if (!isalnum(*token) && *token != ',') {
                LogError(CONTEXT, "HandleDataLabel", "Invalid character in .data directive");
                return STATUS_ERROR;
            }

            char *token = strtok(token, ",");
            while (*token) {
                values++;
                token = strtok(NULL, ",");
            }
        }
        return values;
    }

    if (strncmp(token, ISTRING, strlen(ISTRING)) == 0) {
        if (*token != '\"') {
            LogError(CONTEXT, "HandleDataLabel", "Missing opening quote in .string directive");
            return STATUS_ERROR;
        }
        token++;

        int values = 0;
        while (isalpha(*token) && *token != '\"') {
            values++;
            token++;
        }

        return values;
    }

    LogError(CONTEXT, "HandleDataLabel", "Unknown directive type");
    return STATUS_ERROR;
}

/* First pass: builds the symbol table */
int BuildSymbolTable(char *file_path, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_path || !labels || !label_count) {
        LogError(CONTEXT, "BuildSymbolTable", "Received NULL input");
        return STATUS_ERROR;
    }

    FILE *file_fd = fopen(file_path, "r");
    if (!file_fd) {
        LogError(CONTEXT, "BuildSymbolTable", "Failed to open file");
        return STATUS_ERROR;
    }

    char line[MAX_LINE_LENGTH] = {0};
    char *entries[MAX_LABELS]  = {0};
    size_t entry_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        printf("L:%s", line);
        char *trimmed = TrimWhitespace(line);
        if (!trimmed || *trimmed == '\0') continue; // Skip empty lines

        // Handle .entry directive
        if (strncmp(trimmed, IENTRY, strlen(IENTRY)) == 0) {
            char *entry_label = TrimWhitespace(trimmed + strlen(IENTRY));
            if (!entry_label || *entry_label == '\0') {
                LogError(CONTEXT, "BuildSymbolTable", "Missing label in .entry directive");
                continue;
            }

            Label *existing = FindLabel(entry_label, labels, label_count);
            if (existing) {
                existing->entr = 1;
            } else {
                LogError(CONTEXT, "BuildSymbolTable", "Undefined .entry label (will check in second pass)");
                entries[entry_count] = strndup(entry_label, strlen(entry_label));
                entry_count++;
            }
            continue;
        }

        // Handle .extern directive
        if (strncmp(trimmed, IEXTERN, strlen(IEXTERN)) == 0) {
            char *extern_label = TrimWhitespace(trimmed + strlen(IEXTERN));
            if (!extern_label || *extern_label == '\0') {
                LogError(CONTEXT, "BuildSymbolTable", "Missing label in .extern directive");
                continue;
            }

            Label *existing = FindLabel(extern_label, labels, label_count);
            if (existing) {
                LogError(CONTEXT, "BuildSymbolTable", "Label redefined as .extern");
                continue;
            }

            labels[*label_count].name = strndup(extern_label, strlen(extern_label));
            labels[*label_count].address = 0;
            labels[*label_count].extr = 1;
            (*label_count)++;

            continue;
        }

        // Handle Label Definitions
        Label *curr = malloc(sizeof(Label));
        if (!curr) {
            LogError(CONTEXT, "BuildSymbolTable", "Memory allocation failed for label");
            fclose(file_fd);
            return STATUS_ERROR;
        }

        int status = AddLabel(line, curr);
        if (status == STATUS_NO_RESULT) {
            free(curr);
            continue;
        } else if (status == STATUS_ERROR) {
            LogError(CONTEXT, "BuildSymbolTable", "Error parsing label");
            free(curr);
            continue;
        }

        // Process label if found
        char *rest = strchr(trimmed, ':');
        printf("line:%s/rest:%s", line, rest);
        if (!rest) {
            LogError(CONTEXT, "BuildSymbolTable", "Failed to find colon in label declaration");
            printf("Trimmed: %s / Line: %s", trimmed, line);
            free(curr);
            return STATUS_ERROR;
        }
        rest++;

        if (FindLabel(curr->name, labels, label_count) != NULL) {
            LogError(CONTEXT, "BuildSymbolTable", "Duplicate label definition");
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
                    IC += words; // Forward Instruction Counter
                } else {
                    LogError(CONTEXT, "BuildSymbolTable", "Illegal command parameters in label");
                }
            } else {
                LogError(CONTEXT, "BuildSymbolTable", "Invalid command in label definition");
            }
        }

        // Store label
        labels[*label_count] = *curr;
        (*label_count)++;

        free(curr);
    }

    // Re-check .entry labels
    for (size_t i = 0; i < entry_count; i++) {
        Label *entry = FindLabel(entries[i], labels, label_count);
        if (!entry) {
            LogError(CONTEXT, "BuildSymbolTable", "Undefined .entry label in second pass");
        } else {
            entry->entr = 1; // Mark as an entry
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
