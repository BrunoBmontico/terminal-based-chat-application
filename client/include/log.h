#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef enum
{
    INFO,
    WARN,
    ERROR,
} LogLevel;

void log_message(LogLevel level, const char *message);

#define LOG(...) log_message(__VA_ARGS__)

#endif