#include "Logging.h"

#include <stdarg.h>
#include <stdio.h>

static void log_vprint(const char *prefix, const char *format, va_list args)
{
    fprintf(stderr, "%s", prefix);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

void log_info(const char *message)
{
    if (message != NULL)
    {
        fprintf(stderr, "%s\n", message);
    }
}

void log_error(const char *message)
{
    if (message != NULL)
    {
        fprintf(stderr, "%s\n", message);
    }
}

void log_infof(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_vprint("", format, args);
    va_end(args);
}

void log_errorf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_vprint("", format, args);
    va_end(args);
}
