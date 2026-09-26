#pragma once
#include "../gl/IndexBuffer.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"
#include "Frustum.h"

#include <glm.hpp>

#include <vector>

/// Interleaved vertex format shared by every mesh (locations 0 to 4).
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 texCoords;
};

/**
 * Indexed triangle geometry living on the GPU.
 *
 * A mesh only owns geometry. The material it is drawn with is chosen by the
 * MeshRenderer component, so the same mesh can be reused with several
 * materials.
 */
class Mesh {
public:
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<unsigned int> &indices);

  /// Issues the indexed draw call. The caller binds the shader beforehand.
  void draw() const;

  [[nodiscard]] GLsizei getIndexCount() const { return mIBO.getCount(); }

  /// Bounds in model space, used for culling.
  [[nodiscard]] const AABB &getBounds() const { return mBounds; }

private:
  VertexBuffer mVBO;
  IndexBuffer mIBO;
  VertexArray mVAO;
  AABB mBounds;
};
