#ifndef FREEBREWIE_LOGGING_H
#define FREEBREWIE_LOGGING_H

/** Write a plain informational line to the service log. */
void log_info(const char *message);
/** Write a plain error line to the service log. */
void log_error(const char *message);
/** Write a printf-style informational line to the service log. */
void log_infof(const char *format, ...);
/** Write a printf-style error line to the service log. */
void log_errorf(const char *format, ...);

#endif
