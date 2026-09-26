#pragma once
#include <glm.hpp>

#include <array>

/// Axis aligned bounding box.
struct AABB {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};

  /// Box enclosing this one once transformed by `matrix` (affine).
  [[nodiscard]] AABB transformed(const glm::mat4 &matrix) const;
};

/**
 * The six planes of a view-projection volume, used to skip objects that
 * cannot be visible.
 *
 * Planes are extracted from the matrix (Gribb and Hartmann), so the same
 * class works for the perspective camera and for orthographic light
 * projections. Normals point inwards.
 */
class Frustum {
public:
  enum Plane { Left, Right, Bottom, Top, Near, Far, Count };

  explicit Frustum(const glm::mat4 &viewProjection);

  /// False only when the box is entirely outside one of the planes. It can
  /// return true for boxes that are outside near a corner, which is harmless.
  /// @param ignoreNear Skip the near plane, for shadow casters that are
  ///        clamped onto it rather than clipped
  [[nodiscard]] bool intersects(const AABB &box, bool ignoreNear = false) const;

private:
  std::array<glm::vec4, Count> mPlanes{}; ///< xyz normal, w distance
};
