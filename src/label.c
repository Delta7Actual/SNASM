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

int AddLabel(const char *line, Label *label) {
    if (!line || !label) return STATUS_ERROR;

    // Create a local copy of the line to avoid modifying the original
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';  // Ensure null termination

    // Remove comments by copying only up to COMMENT_CHAR
    char *comment_start = strchr(line_copy, COMMENT_CHAR);
    if (comment_start) *comment_start = '\0';  // Truncate at comment

    // Trim whitespace
    char *trimmed = TrimWhitespace(line_copy);
    if (!trimmed || *trimmed == '\0') return STATUS_NO_RESULT;  // No label found

    // Locate colon (`:`) for label declaration
    char *colon = strchr(trimmed, LABEL_DELIM);
    if (!colon) return STATUS_NO_RESULT;  // Not a label declaration

    // Extract label name without modifying `line`
    size_t label_length = colon - trimmed;
    if (label_length > MAX_LABEL_NAME) return STATUS_ERROR;  // Prevent buffer overflow

    char label_name[MAX_LABEL_NAME + 1];
    strncpy(label_name, trimmed, label_length);
    label_name[label_length] = '\0';

    // Validate label name
    if (ValidateLabelName(label_name) < 0) return STATUS_ERROR;

    // Allocate memory for label name
    label->name = strndup(label_name, label_length);
    if (!label->name) return STATUS_ERROR;  // Memory allocation failed

    // Determine label type
    label->type = DetermineLabelType(colon + 1);
    return 0;
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