#pragma once
#include <glad/glad.h>
#include <glm.hpp>

#include <cstddef>
#include <string>

/**
 * Everything the C++ side and the shaders must agree on: uniform block
 * layouts, binding points, texture units and array sizes.
 *
 * The constants are injected into every shader as #defines (see defines()),
 * so GLSL never hardcodes them. The structs mirror the std140 blocks in
 * res/shaders/common/ and only use vec4/mat4/ivec4 members, which have
 * the same layout in C++ and in std140 without manual padding.
 */
namespace ShaderInterface {

// Uniform block binding points
constexpr GLuint kFrameBlock = 0;
constexpr GLuint kLightsBlock = 1;
constexpr GLuint kShadowBlock = 2;

// Texture units
constexpr GLuint kShadowMapUnit = 0;
constexpr GLuint kAlbedoUnit = 1;
constexpr GLuint kSpecularUnit = 2;
constexpr GLuint kNormalUnit = 3;
constexpr GLuint kSkyboxUnit = 4;
constexpr GLuint kScreenUnit = 5;

// Array sizes
constexpr int kCascadeCount = 4;
constexpr int kMaxPointLights = 64;
constexpr int kMaxSpotLights = 16;

/// `layout(std140, binding = FRAME_BLOCK) uniform FrameBlock`
struct FrameUniforms {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
  glm::vec4 cameraPosition; ///< xyz position, w unused
  glm::vec4 viewport;       ///< width, height, near plane, far plane
};

struct DirectionalLightUniforms {
  glm::vec4 direction; ///< xyz direction the light travels in, w unused
  glm::vec4 radiance;  ///< rgb color * intensity, w unused
  glm::vec4 ambient;   ///< rgb ambient term of the scene, w unused
};

struct PointLightUniforms {
  glm::vec4 positionRange; ///< xyz world position, w range
  glm::vec4 radiance;      ///< rgb color * intensity, w unused
};

struct SpotLightUniforms {
  glm::vec4 positionRange;     ///< xyz world position, w range
  glm::vec4 directionOuterCos; ///< xyz direction, w cos(outer half angle)
  glm::vec4 radianceInnerCos;  ///< rgb color * intensity, w cos(inner)
};

/// `layout(std140, binding = LIGHTS_BLOCK) uniform LightsBlock`
struct LightUniforms {
  DirectionalLightUniforms sun;
  glm::ivec4 counts; ///< x point lights, y spot lights, z has sun
  PointLightUniforms pointLights[kMaxPointLights];
  SpotLightUniforms spotLights[kMaxSpotLights];
};

/// `layout(std140, binding = SHADOW_BLOCK) uniform ShadowBlock`
struct ShadowUniforms {
  glm::mat4 lightSpaceMatrices[kCascadeCount];
  /// x view space far plane, y world size of a texel, z depth range of a
  /// texel (see ShadowData), w unused
  glm::vec4 cascades[kCascadeCount];
  /// x constant bias, y slope bias (both in texels), z normal offset scale
  glm::vec4 bias;
  /// x shadows enabled, y PCF, z show cascades
  glm::ivec4 flags;
};

// std140 layout checks: sizes must be multiples of 16 bytes and members must
// not be padded by the C++ compiler
static_assert(sizeof(FrameUniforms) == 3 * 64 + 2 * 16);
static_assert(sizeof(PointLightUniforms) == 32);
static_assert(sizeof(SpotLightUniforms) == 48);
static_assert(offsetof(LightUniforms, pointLights) == 64);
static_assert(sizeof(LightUniforms) ==
              64 + kMaxPointLights * 32 + kMaxSpotLights * 48);
static_assert(sizeof(ShadowUniforms) == kCascadeCount * (64 + 16) + 2 * 16);

/// `#define` lines declaring the constants above, for every shader.
std::string defines();

} // namespace ShaderInterface
