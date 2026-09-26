#include "Frustum.h"

AABB AABB::transformed(const glm::mat4 &matrix) const {
  // Arvo's method: transform the center, and grow the half extent by the
  // absolute value of each axis of the matrix.
  const glm::vec3 center = (min + max) * 0.5f;
  const glm::vec3 extent = (max - min) * 0.5f;

  const glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));
  glm::vec3 newExtent(0.0f);
  for (int axis = 0; axis < 3; ++axis)
    newExtent += glm::abs(glm::vec3(matrix[axis])) * extent[axis];

  return {newCenter - newExtent, newCenter + newExtent};
}

Frustum::Frustum(const glm::mat4 &m) {
  // Rows of the matrix (GLM is column-major)
  const glm::vec4 row0(m[0][0], m[1][0], m[2][0], m[3][0]);
  const glm::vec4 row1(m[0][1], m[1][1], m[2][1], m[3][1]);
  const glm::vec4 row2(m[0][2], m[1][2], m[2][2], m[3][2]);
  const glm::vec4 row3(m[0][3], m[1][3], m[2][3], m[3][3]);

  // A point is inside when -w <= x, y, z <= w in clip space
  mPlanes[Left] = row3 + row0;
  mPlanes[Right] = row3 - row0;
  mPlanes[Bottom] = row3 + row1;
  mPlanes[Top] = row3 - row1;
  mPlanes[Near] = row3 + row2;
  mPlanes[Far] = row3 - row2;

  for (auto &plane : mPlanes)
    plane /= glm::length(glm::vec3(plane));
}

bool Frustum::intersects(const AABB &box, const bool ignoreNear) const {
  for (int i = 0; i < Count; ++i) {
    if (ignoreNear && i == Near)
      continue;

    // Corner of the box furthest along the plane normal
    const glm::vec3 normal(mPlanes[i]);
    const glm::vec3 corner(normal.x >= 0.0f ? box.max.x : box.min.x,
                           normal.y >= 0.0f ? box.max.y : box.min.y,
                           normal.z >= 0.0f ? box.max.z : box.min.z);
    if (glm::dot(normal, corner) + mPlanes[i].w < 0.0f)
      return false;
  }
  return true;
}
