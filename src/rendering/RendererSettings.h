#pragma once
#include <cstddef>

/// Tweakable renderer options, edited live from the debug UI.
struct RendererSettings {
  // Shadows
  bool shadowsEnabled = true;
  /// Blend between logarithmic (1) and uniform (0) cascade splits.
  float cascadeSplitLambda = 0.8f;
  /// Depth bias in texels, constant part and part scaled by the slope.
  float shadowBiasConstant = 1.0f;
  float shadowBiasSlope = 2.0f;
  /// Multiplier of the normal offset (about one texel of the cascade).
  float shadowNormalOffset = 1.0f;
  bool shadowPcf = true;

  // Lights
  bool localLightsEnabled = true; ///< Point and spot lights

  // Performance
  bool frustumCulling = true;

  // Debug views
  bool showCascades = false;
  bool showLightGizmos = true;
};

/// What the last frame did, for the debug UI.
struct RenderStats {
  size_t submitted = 0;     ///< Meshes in the scene
  size_t drawnLighting = 0; ///< Meshes drawn by the lighting pass
  size_t drawnShadow = 0;   ///< Meshes drawn into at least one cascade
  int pointLights = 0;
  int spotLights = 0;
};
