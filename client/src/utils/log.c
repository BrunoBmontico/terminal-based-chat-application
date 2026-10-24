#include <stdio.h>
#include <time.h>
#include <log.h>

static const char *level_names[] = {
    "INFO",
    "WARN",
    "ERROR"
};

static char* timestamp(char *buf, size_t buf_size)
{
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    strftime(buf, buf_size, "%H:%M:%S", &tm_info);
    return buf;
}

void log_message(LogLevel level, const char *message)
{
    char time_buf[16];

    fprintf(
        stderr,
        "[%s|%s] %s\n",
        timestamp(time_buf,sizeof(time_buf)),
        level_names[level],
        message
    );
}