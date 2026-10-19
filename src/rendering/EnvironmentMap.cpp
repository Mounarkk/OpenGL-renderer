#include "EnvironmentMap.h"
#include "../core/Config.h"
#include "../core/Logger.h"
#include "../gl/Debug.h"
#include "../resource/ResourceManager.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>

namespace {
// Cube faces get a quarter of the panorama width, which keeps its
// resolution around the horizon, within these bounds
constexpr int kMinCubeMapSize = 64;
constexpr int kMaxCubeMapSize = 1024;

constexpr GLuint kComputeLocalSize = 8;

int mipLevels(const int size) {
  return 1 + static_cast<int>(std::floor(std::log2(size)));
}
} // namespace

EnvironmentMap::EnvironmentMap(const std::string &path) {
  GLDebugGroup group("Environment precomputation");
  const auto start = std::chrono::steady_clock::now();

  const bool isPanorama = std::filesystem::path(path).extension() == ".hdr";
  const bool loaded = isPanorama ? loadPanorama(path) : loadFaces(path);
  if (!loaded) {
    Logger::get()->error("Could not load the environment {}, using a uniform "
                         "sky instead",
                         path);
    createUniformSky();
  }

  glGenerateTextureMipmap(mCubeMap);
  glTextureParameteri(mCubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(mCubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Only to report the time spent, the GPU work is otherwise asynchronous
  glFinish();
  const auto elapsed = std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - start);
  Logger::get()->info("Environment {} ready in {:.0f} ms ({}x{} faces)", path,
                      elapsed.count(), mSize, mSize);
}

EnvironmentMap::~EnvironmentMap() {
  if (mCubeMap != 0 && glfwGetCurrentContext())
    glDeleteTextures(1, &mCubeMap);
}

void EnvironmentMap::createCubeMap(const int size) {
  mSize = size;
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mCubeMap);
  glTextureStorage2D(mCubeMap, mipLevels(size), GL_RGBA16F, size, size);
}

bool EnvironmentMap::loadPanorama(const std::string &path) {
  // The conversion shader expects the top row first
  stbi_set_flip_vertically_on_load(false);
  int width, height, channels;
  float *data = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
  if (!data)
    return false;

  // 32-bit floats: a sun in the panorama can exceed the 16-bit range
  GLuint panorama = 0;
  glCreateTextures(GL_TEXTURE_2D, 1, &panorama);
  glTextureStorage2D(panorama, 1, GL_RGB32F, width, height);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTextureSubImage2D(panorama, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  stbi_image_free(data);

  // Longitudes wrap around, latitudes stop at the poles
  glTextureParameteri(panorama, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(panorama, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(panorama, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(panorama, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  createCubeMap(std::clamp(width / 4, kMinCubeMapSize, kMaxCubeMapSize));

  const auto shader = ResourceManager::loadComputeShader(
      Config::getShaderPath() + "ibl/panorama_to_cubemap.comp");
  glBindTextureUnit(0, panorama);
  glBindImageTexture(0, mCubeMap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  const auto size = static_cast<GLuint>(mSize);
  shader->dispatch(size, size, 6, kComputeLocalSize, kComputeLocalSize);

  // The mipmaps are built from what the shader wrote
  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
  glDeleteTextures(1, &panorama);
  return true;
}

bool EnvironmentMap::loadFaces(const std::string &directory) {
  // Order of the cube map layers, +X, -X, +Y, -Y, +Z, -Z
  const std::array<std::string, 6> faces = {"right.jpg", "left.jpg",
                                            "top.jpg",   "bottom.jpg",
                                            "front.jpg", "back.jpg"};

  // Cube map faces follow the image convention (top row first). stb_image
  // converts 8-bit images to linear floats with a 2.2 gamma.
  stbi_set_flip_vertically_on_load(false);
  for (size_t face = 0; face < faces.size(); ++face) {
    const std::string path =
        (std::filesystem::path(directory) / faces[face]).string();
    int width, height, channels;
    float *data = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
    if (!data || width != height || (mCubeMap != 0 && width != mSize)) {
      Logger::get()->error("Invalid cube map face {}", path);
      stbi_image_free(data);
      if (mCubeMap != 0)
        glDeleteTextures(1, &mCubeMap);
      mCubeMap = 0;
      return false;
    }

    if (mCubeMap == 0)
      createCubeMap(width);
    glTextureSubImage3D(mCubeMap, 0, 0, 0, static_cast<GLint>(face), width,
                        height, 1, GL_RGB, GL_FLOAT, data);
    stbi_image_free(data);
  }

  // Large faces are cut down to the first mip level that fits: lighting
  // gains nothing from them and a 2048 float cube map takes 270 MB
  int skippedLevels = 0;
  while ((mSize >> skippedLevels) > kMaxCubeMapSize)
    ++skippedLevels;
  if (skippedLevels > 0) {
    glGenerateTextureMipmap(mCubeMap);
    const GLuint full = mCubeMap;
    createCubeMap(mSize >> skippedLevels);
    glCopyImageSubData(full, GL_TEXTURE_CUBE_MAP, skippedLevels, 0, 0, 0,
                       mCubeMap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, mSize, mSize,
                       6);
    glDeleteTextures(1, &full);
  }
  return true;
}

void EnvironmentMap::createUniformSky() {
  constexpr float kGrey[] = {0.5f, 0.5f, 0.5f, 1.0f};
  createCubeMap(kMinCubeMapSize);
  glClearTexImage(mCubeMap, 0, GL_RGBA, GL_FLOAT, kGrey);
}
