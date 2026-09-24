#pragma once

#include <string>
#include <unordered_map>

/**
 * Key/value settings read from config.txt.
 *
 * Lines use the `key = value` format and `#` starts a comment. Every getter
 * falls back to a sane default when the key is missing or malformed, so the
 * renderer still starts without a config file.
 *
 * Relative paths are resolved against the project root (set at build time
 * through RENDERER_ROOT_DIR), which makes the executable independent of the
 * working directory it is launched from.
 */
class Config {
public:
  /**
   * Parses the given file. Missing files are not an error: defaults are used.
   * @param configPath Path to the configuration file
   * @return true if the file was found and parsed
   */
  static bool load(const std::string &configPath);

  /// Absolute path of the project root, with a trailing slash.
  static std::string getRootPath();

  /// Resolves a path relative to the project root. Absolute paths are kept.
  static std::string resolvePath(const std::string &path);

  static int getWindowWidth();
  static int getWindowHeight();
  static bool getVSyncEnabled();

  static std::string getShaderPath();
  static std::string getModelPath();
  static std::string getTexturePath();
  static std::string getSkyboxPath();
  static std::string getScreenshotPath();

  static float getCameraFOV();
  static float getCameraSpeed();
  static float getCameraSensitivity();
  static float getCameraNearPlane();
  static float getCameraFarPlane();

  /// Resolution (width and height) of each shadow cascade.
  static int getShadowMapSize();

private:
  static std::unordered_map<std::string, std::string> s_ConfigValues;

  static std::string getString(const std::string &key,
                               const std::string &defaultValue);
  static int getInt(const std::string &key, int defaultValue);
  static float getFloat(const std::string &key, float defaultValue);
  static bool getBool(const std::string &key, bool defaultValue);
};
