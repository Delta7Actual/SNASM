#include "../include/firstpass.h"
#include "../include/logger.h"

#include "../include/firstpass.h"

int  HandleDSDirective(char *token) {
    if (!token) return STATUS_ERROR;

    // Skip leading spaces
    while (isspace((unsigned char)*token)) token++;

    // Handling .data directive
    if (strncmp(token, IDATA, strlen(IDATA)) == 0) {
        token += strlen(IDATA);
        while (isspace((unsigned char)*token)) token++;  // Skip spaces after ".data"

        int values = 0;
        int expect_number = 1;  // Expecting a number first

        while (*token != '\n' && *token != '\0') {
            if (isdigit(*token) || (*token == POS_DELIM || *token == NEG_DELIM)) {
                if (*token == POS_DELIM || *token == NEG_DELIM) {  
                    token++;  // Move past sign
                    if (!isdigit(*token)) return STATUS_ERROR;  // Sign must be followed by a number
                }

                values++;  // Count valid numbers
                expect_number = 0;  // Now expecting a comma or end

                while (isdigit(*token)) token++;  // Skip full number
                while (isspace((unsigned char)*token)) token++;  // Skip spaces
            } 
            else if (*token == ',') {
                if (expect_number) return STATUS_ERROR;  // Invalid if ",," or leading ","
                expect_number = 1;  // Now expecting a number
                token++;  // Move past comma
                while (isspace((unsigned char)*token)) token++;  // Skip spaces
            } 
            else {
                return STATUS_ERROR;  // Unexpected character
            }
        }

        return expect_number ? STATUS_ERROR : values;  // Ensure last token was valid
    }

    // Handling .string directive
    if (strncmp(token, ISTRING, strlen(ISTRING)) == 0) {
        token += strlen(ISTRING);
        while (isspace((unsigned char)*token)) token++;  // Skip spaces after ".string"

        if (*token != '\"') return STATUS_ERROR;  // Ensure opening quote
        token++;  // Skip opening quote

        int values = 0;
        while (*token && *token != '\"') {  // Read until closing quote
            values++;
            token++;
        }

        if (*token != '\"') return STATUS_ERROR;  // Ensure closing quote exists
        return values + 1;  // +1 for null terminator
    }

    return STATUS_ERROR;
}

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
                int isExternInFile = 0;
                for (size_t i = 0; i < extern_count; i++) {
                    if (strncmp(existing->name, externals[i], strlen(existing->name)) == 0) {
                        printf("(-) Label cannot be defined as both extern and entry in the same file! -> %s"
                            , existing->name);
                        isExternInFile = 1;
                        break;
                    }
                }
                if (isExternInFile == 0) existing->extr = 1;
            } else {
                entries[entry_count] = strndup(ptr, strlen(ptr)-1);
                entry_count++;
                printf("\n(*) .entry label not defined (Will check in second pass) -> %s\n\n", entries[entry_count-1]);
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

            memset(&labels[*label_count], 0, sizeof(Label));
            labels[*label_count].name = strndup(ptr, strlen(ptr));
            labels[*label_count].address = 0;
            labels[*label_count].extr = 1;
            (*label_count)++;

            externals[extern_count] = strndup(ptr, strlen(ptr));
            extern_count++;

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
                int values = HandleDSDirective(line);
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
            int values =  HandleDSDirective(rest);
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

int ValidateSymbolTable(Label labels[MAX_LABELS], size_t *label_count) {
    if (!labels || !label_count) return STATUS_ERROR;

    int status = 0;
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].entr != labels[i].extr) {
            status = -1;
            printf("\n(-) Illegal label: Failed to find matching %s for label %s in program!\n\n", 
            (labels[i].entr == 0) ? ".entry" : ".extern", labels[i].name);
        }
    }

    return status;
}

/* Generates the `.ent` file */
int FormatEntryFile(char *file_name, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_name || !labels || !label_count) return STATUS_ERROR;

    FILE *output_fd = fopen(file_name, "w");
    if (!output_fd) return -1;
    
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].entr) {
            fprintf(output_fd, "%s: %07d\n", labels[i].name, labels[i].address);
        }
    }

    fclose(output_fd);
    return 0;
}

/* Generates the `.ext` file */
int FormatExternalFile(char *file_name, Label labels[MAX_LABELS], size_t *label_count) {
    if (!file_name || !label_count || !label_count) return STATUS_ERROR;

    FILE *output_fd = fopen(file_name, "w");
    if (!output_fd) return -1;
    
    for (size_t i = 0; i < *label_count; i++) {
        if (labels[i].extr) {
            fprintf(output_fd, "%s: %07d\n", labels[i].name, labels[i].address);
        }
    }

    fclose(output_fd);
    return 0;
}