#include <components/percona_telemetry/logger.h>

Logger::Logger(SERVICE_TYPE(log_builtins) &,
               SERVICE_TYPE(log_builtins_string) &, loglevel) {}

void Logger::info(const char *, ...) {}
void Logger::warning(const char *, ...) {}
void Logger::error(const char *, ...) {}
