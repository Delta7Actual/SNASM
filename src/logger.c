#include "../include/logger.h"

LogLevel CURRENT_LOG_LEVEL = LOG_NORMAL;

#define ANSI_DIM    "\033[2m"
#define ANSI_RESET  "\033[0m"

void SetLogLevel(LogLevel level) {
    CURRENT_LOG_LEVEL = level;
}

void LogInfo(const char *fmt, ...) {
    if (CURRENT_LOG_LEVEL >= LOG_NORMAL) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

void LogVerbose(const char *fmt, ...) {
    if (CURRENT_LOG_LEVEL >= LOG_VERBOSE) {
        printf("%s- %s", ANSI_DIM, ANSI_RESET);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

void LogDebug(const char *fmt, ...) {
    if (CURRENT_LOG_LEVEL >= LOG_DEBUG) {
        printf("%s[DEBUG]: %s", ANSI_DIM, ANSI_RESET);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

void LogU32AsBin(uint32_t num) {
    num = WORD(num);
    for (int i = 23; i >= 0; i--) {
        putchar((num & (1 << i)) ? '1' : '0');
        if (i % 4 == 0 && i != 0) putchar(' '); // Group by 4 bits
    }
    putchar('\n');
}