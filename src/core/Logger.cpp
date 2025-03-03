#include "Logger.h"
std::shared_ptr<spdlog::logger> Logger::sLogger;

void Logger::init() {
  spdlog::set_pattern("%^[%T] %n: %v%$");
  sLogger = spdlog::stdout_color_mt("APP");
  sLogger->set_level(spdlog::level::trace);
}
