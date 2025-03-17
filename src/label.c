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

int AddLabel(FILE *file_fd, Label *label) {
    if (!file_fd || !label) return STATUS_ERROR;

    char line[MAX_LINE_LENGTH] = {0};

    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        char *comment_start = strchr(line, COMMENT_CHAR);
        if (comment_start) *comment_start = '\0'; // We dont care about the rest

        char *trimmed = TrimWhitespace(line);
        if (!trimmed || *trimmed == '\0') continue;

        char *colon = strchr(trimmed, LABEL_DELIM);
        if (!colon) continue;

        *colon = '\0';
        char *label_name = TrimWhitespace(trimmed);
        
        int name_len = ValidateLabelName(label_name);
        if (name_len < 0) return STATUS_ERROR;

        label->name = strndup(label_name, name_len + 1);
        label->type = DetermineLabelType(colon + 1);

        return 0;
    }
    
    return STATUS_NO_RESULT;
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