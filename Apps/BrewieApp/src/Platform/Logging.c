#include "Logging.h"

#include <stdarg.h>
#include <stdio.h>

/*
 * systemd captures stdout/stderr from brewie.service into the journal. Keeping logging as a
 * tiny wrapper makes the rest of the app independent from that detail, and leaves room to
 * redirect logs later without changing every call site.
 */
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
