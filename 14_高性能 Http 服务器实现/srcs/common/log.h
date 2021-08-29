#ifndef LOG_H
#define LOG_H

#include "stdarg.h"

#define LOG_DEBUG_TYPE 0
#define LOG_MSG_TYPE 1
#define LOG_WARN_TYPE 2
#define LOG_ERR_TYPE 3

void log(int severity, const char *msg);
void logx(int severity, const char *errstr, const char *fmt, va_list ap);
void msgx(const char *fmt, ...);
void debugx(const char *fmt, ...);

#define LOG_MSG(msg) log(LOG_MSG_TYPE, msg)
#define LOG_ERR(msg) log(LOG_ERR_TYPE, msg)

#endif