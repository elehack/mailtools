#include <stdio.h>
#include <stdarg.h>

#include "logging.h"

int log_level;

void log_debug(const char *fmt, ...)
{
    va_list ap;
    if (log_level <= LOG_DEBUG) {
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fputc('\n', stderr);
    }
}

void log_info(const char *fmt, ...)
{
    va_list ap;
    if (log_level <= LOG_NORMAL) {
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fputc('\n', stderr);
    }
}

void log_warning(const char *fmt, ...)
{
    va_list ap;
    fputs("warning: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void log_error(const char *fmt, ...)
{
    va_list ap;
    fputs("error: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

