#pragma once
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/spdlog.h>

class Logger {
public:
  static void init();
  static std::shared_ptr<spdlog::logger> &get() { return sLogger; }

private:
  static std::shared_ptr<spdlog::logger> sLogger;
};
