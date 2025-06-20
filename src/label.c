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

    // Make a local copy of the line to avoid modifying the original
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';  // Ensure null termination

    // Remove comments
    char *comment_start = strchr(line_copy, COMMENT_DELIM);
    if (comment_start) *comment_start = '\0';  // Truncate at comment

    // Skip leading whitespace manually
    char *ptr = line_copy;
    while (isspace((unsigned char)*ptr)) ptr++;  

    if (*ptr == '\0') return STATUS_NO_RESULT;  // Empty line

    // Locate colon (`:`) for label declaration
    char *colon = strchr(ptr, LABEL_DELIM);
    if (!colon) return STATUS_NO_RESULT;  // Not a label declaration

    // Find the label name length while ensuring it's valid
    size_t label_length = colon - ptr;
    if (label_length == 0 || label_length > MAX_LABEL_NAME) return STATUS_ERROR;  

    // Copy the label name without modifying `line`
    char label_name[MAX_LABEL_NAME + 1];
    strncpy(label_name, ptr, label_length);
    label_name[label_length] = '\0';

    // Validate label name
    if (ValidateLabelName(label_name) < 0) return STATUS_ERROR;

    // Allocate memory for label name
    label->name = strndup(label_name, label_length);
    if (!label->name) return STATUS_ERROR;  // Memory allocation failed

    // Skip whitespace after the colon to find label type
    ptr = colon + 1;
    while (isspace((unsigned char)*ptr)) ptr++;  

    // Determine label type
    label->type = DetermineLabelType(ptr);
    
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
    LogDebug("Cleaning up label...");
    if (label == NULL) return STATUS_ERROR;
    if (label->name) free(label->name);
    free(label);
    return 0;
}