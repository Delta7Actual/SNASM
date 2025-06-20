#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>

#include "definitions.h"

typedef enum log_level_e {
    LOG_QUIET,
    LOG_NORMAL,
    LOG_VERBOSE,
    LOG_DEBUG
} LogLevel;

extern LogLevel CURRENT_LOG_LEVEL;

void SetLogLevel(LogLevel level);
void LogInfo(const char *fmt, ...);
void LogVerbose(const char *fmt, ...);
void LogDebug(const char *fmt, ...);

void LogU32AsBin(uint32_t num);

#endif