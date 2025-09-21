#include "Config.h"
#include "Logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>

// Static member definitions
std::unordered_map<std::string, std::string> Config::s_ConfigValues;
bool Config::s_Loaded = false;

bool Config::load(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        Logger::get()->warn("Could not open config file: {}", configPath);
        Logger::get()->info("Using default configuration values");
        s_Loaded = true; // Still mark as loaded to use defaults
        return false;
    }

    s_ConfigValues.clear();
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the '=' separator
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            Logger::get()->warn("Invalid config line {} in {}: {}", lineNumber, configPath, line);
            continue;
        }

        // Extract key and value
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        // Trim whitespace from key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (!key.empty()) {
            s_ConfigValues[key] = value;
            Logger::get()->debug("Config: {} = {}", key, value);
        }
    }

    file.close();
    s_Loaded = true;
    Logger::get()->info("Configuration loaded from: {}", configPath);
    return true;
}

int Config::getWindowWidth() {
    return getInt("window.width", 800);
}

int Config::getWindowHeight() {
    return getInt("window.height", 600);
}

std::string Config::getShaderPath() {
    return getString("paths.shaders", "../res/shaders/");
}

std::string Config::getModelPath() {
    return getString("paths.models", "../res/models/");
}

std::string Config::getTexturePath() {
    return getString("paths.textures", "../res/textures/");
}

float Config::getCameraFOV() {
    return getFloat("camera.fov", 45.0f);
}

float Config::getCameraSpeed() {
    return getFloat("camera.speed", 2.5f);
}

float Config::getCameraSensitivity() {
    return getFloat("camera.sensitivity", 0.1f);
}

float Config::getCameraNearPlane() {
    return getFloat("camera.near_plane", 0.1f);
}

float Config::getCameraFarPlane() {
    return getFloat("camera.far_plane", 100.0f);
}

bool Config::getVSyncEnabled() {
    return getBool("renderer.vsync", true);
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) {
    if (!s_Loaded) {
        Logger::get()->warn("Config not loaded, using default for {}", key);
        return defaultValue;
    }

    auto it = s_ConfigValues.find(key);
    if (it != s_ConfigValues.end()) {
        return it->second;
    }
    
    Logger::get()->debug("Config key '{}' not found, using default: {}", key, defaultValue);
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) {
    std::string value = getString(key, std::to_string(defaultValue));
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        Logger::get()->warn("Invalid integer value for {}: '{}', using default: {}", key, value, defaultValue);
        return defaultValue;
    }
}

float Config::getFloat(const std::string& key, float defaultValue) {
    std::string value = getString(key, std::to_string(defaultValue));
    try {
        return std::stof(value);
    } catch (const std::exception& e) {
        Logger::get()->warn("Invalid float value for {}: '{}', using default: {}", key, value, defaultValue);
        return defaultValue;
    }
}

bool Config::getBool(const std::string& key, bool defaultValue) {
    std::string value = getString(key, defaultValue ? "true" : "false");
    
    // Convert to lowercase for comparison
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
    if (value == "true" || value == "1" || value == "yes" || value == "on") {
        return true;
    } else if (value == "false" || value == "0" || value == "no" || value == "off") {
        return false;
    } else {
        Logger::get()->warn("Invalid boolean value for {}: '{}', using default: {}", key, value, defaultValue ? "true" : "false");
        return defaultValue;
    }
}