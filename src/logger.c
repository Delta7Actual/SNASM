#include "../include/logger.h"

void LogError(char *stage, char *func, char *message) {
    if (stage == NULL 
    || func == NULL 
    || message == NULL) printf("--- ERROR IN LOGGER ---\n");

    FILE *logs = fopen("output/logs.txt", "a");
    if (logs == NULL) printf("--- ERROR IN LOGGER ---\n");

    fprintf(logs, "/// %s : %s -> %s", stage, func, message);
}