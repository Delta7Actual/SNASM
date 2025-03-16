#include "../include/logger.h"

void LogError(char *context, char *func, char *message) {
    if (context == NULL 
    || func == NULL 
    || message == NULL) printf("--- ERROR IN LOGGER ---\n");

    FILE *logs = fopen("output/logs.txt", "a");
    if (logs == NULL) printf("--- ERROR IN LOGGER ---\n");

    fprintf(logs, "(-) %s: %s() -> %s", context, func, message);
}