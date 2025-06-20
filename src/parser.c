#include "../include/parser.h"

char *TrimWhitespace(char *str) {
    if (!str) return NULL;
    
    while (isblank((unsigned char)*str)) str++; // Skip leading spaces
    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--; // Remove trailing spaces
    *(end + 1) = '\0'; // Null terminate

    return str;
}

int HandleDSDirective(char *token, uint32_t *data) {
    if (!token) return STATUS_ERROR;

    // Skip leading spaces
    while (isspace((unsigned char)*token)) token++;

    TrimNewline(token);
    LogDebug("Handling DS directive: %s\n", token);
    LogDebug("Currently%s writing to data segment\n", (data) ? "" : " NOT");

    // Handling .data directive
    if (strncmp(token, IDATA, strlen(IDATA)) == 0) {
        LogDebug("Directive is '.data'\n");

        token += strlen(IDATA);
        while (isspace((unsigned char)*token)) token++;  // Skip spaces after ".data"

        int values = 0;
        int expect_number = 1;

        while (*token != '\n' && *token != '\0') {
            if (isdigit(*token) || (*token == POS_DELIM || *token == NEG_DELIM)) {
                char *endptr;
                int number = strtol(token, &endptr, 10);  // Parses sign and digits safely

                if (token == endptr) return STATUS_ERROR;  // Invalid number

                if (data) {
                    data[data[0]] = WORD(number);  // Mask to 24-bit
                    data[0]++;
                }
                values++;

                token = endptr;
                while (isspace((unsigned char)*token)) token++;
                expect_number = 0;
            }
            else if (*token == ',') {
                if (expect_number) return STATUS_ERROR;  // e.g., ",," or starting with ","
                expect_number = 1;
                token++;
                while (isspace((unsigned char)*token)) token++;
            }
            else {
                return STATUS_ERROR;  // Unexpected char
            }
        }

        return expect_number ? STATUS_ERROR : values;
    }

    // Handling .string directive
    if (strncmp(token, ISTRING, strlen(ISTRING)) == 0) {
        LogDebug("Directive is '.string'\n");

        token += strlen(ISTRING);
        while (isspace((unsigned char)*token)) token++;

        if (*token != '\"') return STATUS_ERROR;
        token++;  // Skip opening quote

        int values = 0;
        while (*token && *token != '\"') {
            if (data) {
                data[data[0]] = (uint32_t)(*token) & 0xFF;
                data[0]++;
            }
            values++;
            token++;
        }

        if (*token != '\"') return STATUS_ERROR;

        if (data) {
            data[data[0]] = 0;  // Null terminator
            data[0]++;
        }
        return values + 1;
    }

    return STATUS_ERROR;
}

void TrimNewline(char *line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}