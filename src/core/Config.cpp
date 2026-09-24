#include "Config.h"
#include "Logger.h"

#include <algorithm>
#include <fstream>

#ifndef RENDERER_ROOT_DIR
#define RENDERER_ROOT_DIR "."
#endif

std::unordered_map<std::string, std::string> Config::s_ConfigValues;

namespace {
std::string trim(const std::string &str) {
  const size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  const size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, last - first + 1);
}
} // namespace

bool Config::load(const std::string &configPath) {
  std::ifstream file(configPath);
  if (!file.is_open()) {
    Logger::get()->warn("Could not open config file {}, using defaults",
                        configPath);
    return false;
  }

  s_ConfigValues.clear();
  std::string line;
  int lineNumber = 0;

  while (std::getline(file, line)) {
    lineNumber++;
    line = trim(line);
    if (line.empty() || line[0] == '#')
      continue;

    const size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) {
      Logger::get()->warn("Invalid config line {} in {}: {}", lineNumber,
                          configPath, line);
      continue;
    }

    const std::string key = trim(line.substr(0, equalPos));
    const std::string value = trim(line.substr(equalPos + 1));
    if (!key.empty())
      s_ConfigValues[key] = value;
  }

  Logger::get()->info("Configuration loaded from {}", configPath);
  return true;
}

std::string Config::getRootPath() {
  std::string root = RENDERER_ROOT_DIR;
  if (root.back() != '/')
    root += '/';
  return root;
}

std::string Config::resolvePath(const std::string &path) {
  if (!path.empty() && path[0] == '/')
    return path;
  return getRootPath() + path;
}

int Config::getWindowWidth() { return getInt("window.width", 1280); }

int Config::getWindowHeight() { return getInt("window.height", 720); }

bool Config::getVSyncEnabled() { return getBool("renderer.vsync", true); }

std::string Config::getShaderPath() {
  return resolvePath(getString("paths.shaders", "res/shaders/"));
}

std::string Config::getModelPath() {
  return resolvePath(getString("paths.models", "res/models/"));
}

std::string Config::getTexturePath() {
  return resolvePath(getString("paths.textures", "res/textures/"));
}

std::string Config::getSkyboxPath() {
  return resolvePath(getString("paths.skybox", "res/skyboxes/arctic/"));
}

std::string Config::getScreenshotPath() {
  return resolvePath(getString("paths.screenshots", "screenshots/"));
}

float Config::getCameraFOV() { return getFloat("camera.fov", 45.0f); }

float Config::getCameraSpeed() { return getFloat("camera.speed", 2.5f); }

float Config::getCameraSensitivity() {
  return getFloat("camera.sensitivity", 0.1f);
}

float Config::getCameraNearPlane() {
  return getFloat("camera.near_plane", 0.1f);
}

float Config::getCameraFarPlane() {
  return getFloat("camera.far_plane", 100.0f);
}

int Config::getShadowMapSize() { return getInt("shadow.map_size", 2048); }

std::string Config::getString(const std::string &key,
                              const std::string &defaultValue) {
  const auto it = s_ConfigValues.find(key);
  return it != s_ConfigValues.end() ? it->second : defaultValue;
}

int Config::getInt(const std::string &key, const int defaultValue) {
  const std::string value = getString(key, std::to_string(defaultValue));
  try {
    return std::stoi(value);
  } catch (const std::exception &) {
    Logger::get()->warn("Invalid integer for {}: '{}', using {}", key, value,
                        defaultValue);
    return defaultValue;
  }
}

float Config::getFloat(const std::string &key, const float defaultValue) {
  const std::string value = getString(key, std::to_string(defaultValue));
  try {
    return std::stof(value);
  } catch (const std::exception &) {
    Logger::get()->warn("Invalid float for {}: '{}', using {}", key, value,
                        defaultValue);
    return defaultValue;
  }
}

bool Config::getBool(const std::string &key, const bool defaultValue) {
  std::string value = getString(key, defaultValue ? "true" : "false");
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (value == "true" || value == "1" || value == "yes" || value == "on")
    return true;
  if (value == "false" || value == "0" || value == "no" || value == "off")
    return false;

  Logger::get()->warn("Invalid boolean for {}: '{}', using {}", key, value,
                      defaultValue);
  return defaultValue;
}
