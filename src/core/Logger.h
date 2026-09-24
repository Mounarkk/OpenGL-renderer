#pragma once
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

/// Thin wrapper around a global spdlog logger.
class Logger {
public:
  static void init();
  static std::shared_ptr<spdlog::logger> &get() { return sLogger; }

private:
  static std::shared_ptr<spdlog::logger> sLogger;
};
