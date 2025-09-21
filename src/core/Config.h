#pragma once

#include <string>
#include <unordered_map>

/**
 * Simple configuration system for the renderer.
 *
 * Loads settings from a configuration file and provides access to various
 * renderer settings like window dimensions, camera parameters, and resource
 * paths. Uses a simple key-value format for easy editing and understanding.
 */
class Config {
public:
  /**
   * Loads configuration from the specified file.
   * @param configPath Path to the configuration file
   * @return true if configuration was loaded successfully, false otherwise
   */
  static bool load(const std::string &configPath);

  /**
   * Gets the configured window width.
   * @return Window width in pixels
   */
  static int getWindowWidth();

  /**
   * Gets the configured window height.
   * @return Window height in pixels
   */
  static int getWindowHeight();

  /**
   * Gets the configured shader directory path.
   * @return Path to shader directory
   */
  static std::string getShaderPath();

  /**
   * Gets the configured model directory path.
   * @return Path to model directory
   */
  static std::string getModelPath();

  /**
   * Gets the configured texture directory path.
   * @return Path to texture directory
   */
  static std::string getTexturePath();

  /**
   * Gets the configured camera field of view.
   * @return Camera FOV in degrees
   */
  static float getCameraFOV();

  /**
   * Gets the configured camera movement speed.
   * @return Camera movement speed
   */
  static float getCameraSpeed();

  /**
   * Gets the configured camera mouse sensitivity.
   * @return Camera mouse sensitivity
   */
  static float getCameraSensitivity();

  /**
   * Gets the configured camera near plane distance.
   * @return Near plane distance
   */
  static float getCameraNearPlane();

  /**
   * Gets the configured camera far plane distance.
   * @return Far plane distance
   */
  static float getCameraFarPlane();

  /**
   * Gets whether VSync is enabled.
   * @return true if VSync is enabled, false otherwise
   */
  static bool getVSyncEnabled();

private:
  static std::unordered_map<std::string, std::string> s_ConfigValues;
  static bool s_Loaded;

  /**
   * Gets a string value from the configuration.
   * @param key Configuration key
   * @param defaultValue Default value if key is not found
   * @return Configuration value or default
   */
  static std::string getString(const std::string &key,
                               const std::string &defaultValue);

  /**
   * Gets an integer value from the configuration.
   * @param key Configuration key
   * @param defaultValue Default value if key is not found
   * @return Configuration value or default
   */
  static int getInt(const std::string &key, int defaultValue);

  /**
   * Gets a float value from the configuration.
   * @param key Configuration key
   * @param defaultValue Default value if key is not found
   * @return Configuration value or default
   */
  static float getFloat(const std::string &key, float defaultValue);

  /**
   * Gets a boolean value from the configuration.
   * @param key Configuration key
   * @param defaultValue Default value if key is not found
   * @return Configuration value or default
   */
  static bool getBool(const std::string &key, bool defaultValue);
};