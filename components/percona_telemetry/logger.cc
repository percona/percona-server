#include <stdarg.h>

#include <mysqld_error.h>
#include "logger.h"

SERVICE_TYPE(log_builtins) * log_bi;
SERVICE_TYPE(log_builtins_string) * log_bs;

#define RETURN_IF_BLOCKED(level) \
  if (level > log_level_) return;

namespace {
void log(loglevel level, const char *format, va_list *args)
    MY_ATTRIBUTE((format(printf, 2, 0)));

void log(loglevel level, const char *format, va_list *args) {
  constexpr size_t buffer_len = 8 * 1024;
  char buffer[buffer_len];
  vsnprintf(buffer, buffer_len, format, *args);
  LogComponentErr(level, ER_LOG_PRINTF_MSG, buffer);
}
}  // namespace

Logger::Logger(SERVICE_TYPE(log_builtins) & log_builtins_service,
               SERVICE_TYPE(log_builtins_string) & log_builtins_string_service,
               loglevel log_level)
    : log_level_(log_level) {
  log_bi = &log_builtins_service;
  log_bs = &log_builtins_string_service;
}

void Logger::info(const char *format, ...) {
  RETURN_IF_BLOCKED(INFORMATION_LEVEL);
  va_list args;
  va_start(args, format);
  log(INFORMATION_LEVEL, format, &args);
  va_end(args);
}

void Logger::warning(const char *format, ...) {
  RETURN_IF_BLOCKED(WARNING_LEVEL);
  va_list args;
  va_start(args, format);
  log(WARNING_LEVEL, format, &args);
  va_end(args);
}

void Logger::error(const char *format, ...) {
  RETURN_IF_BLOCKED(ERROR_LEVEL);
  va_list args;
  va_start(args, format);
  log(ERROR_LEVEL, format, &args);
  va_end(args);
}
