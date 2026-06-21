#ifndef FREEBREWIE_LOGGING_H
#define FREEBREWIE_LOGGING_H

void log_info(const char *message);
void log_error(const char *message);
void log_infof(const char *format, ...);
void log_errorf(const char *format, ...);

#endif
