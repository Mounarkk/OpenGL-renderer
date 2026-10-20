#pragma once
#include <glad/glad.h>

#include <string>

/**
 * Light arriving from far away in every direction: the sky and the distant
 * surroundings, stored in a floating point cube map with mipmaps, and the
 * maps image based lighting derives from it.
 *
 * The source is either an equirectangular HDR panorama (.hdr), resampled to
 * a cube map by a compute shader, or a directory holding the six faces of a
 * cube map as right, left, top, bottom, front and back .jpg files. A source
 * that cannot be loaded is replaced by a uniform grey sky, with an error in
 * the log.
 */
class EnvironmentMap {
public:
  explicit EnvironmentMap(const std::string &path);
  ~EnvironmentMap();

  EnvironmentMap(const EnvironmentMap &) = delete;
  EnvironmentMap &operator=(const EnvironmentMap &) = delete;

  /// Radiance of the environment in each direction, in linear HDR values.
  [[nodiscard]] GLuint getCubeMap() const { return mCubeMap; }

  /// Diffuse lighting for each normal direction: irradiance divided by PI,
  /// to multiply with the diffuse color.
  [[nodiscard]] GLuint getIrradianceMap() const { return mIrradianceMap; }

  /// Environment blurred by the specular lobe, one mip level per roughness:
  /// level i is for roughness i / (levels - 1).
  [[nodiscard]] GLuint getPrefilteredMap() const { return mPrefilteredMap; }
  [[nodiscard]] int getPrefilteredLevels() const { return mPrefilteredLevels; }

  /// Scale (R) and bias (G) to apply to f0, for each NdotV (U) and
  /// roughness (V). Independent of the environment.
  [[nodiscard]] GLuint getBrdfLut() const { return mBrdfLut; }

private:
  GLuint mCubeMap = 0;
  GLuint mIrradianceMap = 0;
  GLuint mPrefilteredMap = 0;
  GLuint mBrdfLut = 0;
  int mSize = 0;
  int mPrefilteredLevels = 0;

  /// Allocates mCubeMap with a full mip chain.
  void createCubeMap(int size);

  bool loadPanorama(const std::string &path);
  bool loadFaces(const std::string &directory);
  void createUniformSky();

  void computeIrradiance();
  void computePrefilteredMap();
  void computeBrdfLut();
};
