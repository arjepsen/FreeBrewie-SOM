#ifndef FREEBREWIE_LOGGING_H
#define FREEBREWIE_LOGGING_H

/****************************************************************************************
 * @file Logging.h
 * @brief Small logging interface for service and bring-up messages.
 *
 * Responsibility: provide simple app logging.
 * Owns: plain and printf-style info/error log calls.
 * Does not own: app state or retry/recovery policy.
 ****************************************************************************************/

/** Write a plain informational line to the service log. */
void log_info(const char *message);
/** Write a plain error line to the service log. */
void log_error(const char *message);
/** Write a printf-style informational line to the service log. */
void log_infof(const char *format, ...);
/** Write a printf-style error line to the service log. */
void log_errorf(const char *format, ...);

#endif
