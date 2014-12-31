#ifndef MF_LOGGING_H
#define MF_LOGGING_H

enum log_level {
    LOG_DEBUG,
    LOG_NORMAL,
    LOG_QUIET
};

extern int log_level;

void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif
