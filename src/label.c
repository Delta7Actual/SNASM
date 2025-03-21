#include "../include/label.h"

LType DetermineLabelType(char *token);
int     ValidateLabelName(char *name);

Label *FindLabel(char *name, Label labels[MAX_LABELS], size_t *label_count) {
    if (name == NULL || labels == NULL || label_count == NULL) return NULL;

    for (size_t i = 0; i < *label_count; i++) {
        if (strncmp(labels[i].name, name, strlen(labels[i].name)) == 0) {
            return &labels[i];
        }
    }
    return NULL;
}

int AddLabel(char *line, Label *label) {
    if (!line || !label) return STATUS_ERROR;

    char *temp = strndup(line, strlen(line));

    // Remove comments
    char *comment_start = strchr(temp, COMMENT_CHAR);
    if (comment_start) *comment_start = '\0';

    // Trim whitespace
    char *trimmed = TrimWhitespace(temp);
    if (!trimmed || *trimmed == '\0') return STATUS_NO_RESULT; // Empty line

    // Check if the line contains a label (ends with ':')
    char *colon = strchr(trimmed, LABEL_DELIM);
    if (!colon) return STATUS_NO_RESULT; // No label found

    *colon = '\0'; // Split label from the rest of the line
    char *label_name = TrimWhitespace(trimmed);

    // Validate label name
    int name_len = ValidateLabelName(label_name);
    if (name_len < 0) return STATUS_ERROR;

    // Store label
    label->name = strndup(label_name, name_len + 1);
    if (!label->name) return STATUS_ERROR;

    // Determine label type (CODE or DATA)
    label->type = DetermineLabelType(colon + 1);
    
    return 0; // Successfully processed label
}


LType DetermineLabelType(char *token) {
    if (strncmp(token, ISTRING, strlen(ISTRING)) == 0 
    || strncmp(token, IDATA, strlen(IDATA)) == 0) {
        return E_DATA;
    }
    return E_CODE;
}

// Returns length or ERRORCODE
int ValidateLabelName(char *name) {
    if (!name) return STATUS_ERROR;

    size_t len = strlen(name);
    if (len > 31) return STATUS_ERROR;
    if (!isalpha(name[0])) return STATUS_ERROR;
    
    size_t offset = 1;
    while(offset < len) {
        if (!isalnum(name[offset])) return STATUS_ERROR;
        offset++;
    }

    return len;
}

int CleanUpLabel(Label *label) {
    if (label == NULL) return STATUS_ERROR;
    if (label->name) free(label->name);
    free(label);
    return 0;
}