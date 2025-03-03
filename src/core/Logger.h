#pragma once
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

class Logger {
public:
  static void init();
  static std::shared_ptr<spdlog::logger>& get() { return sLogger; }
private:
  static std::shared_ptr<spdlog::logger> sLogger;
};
