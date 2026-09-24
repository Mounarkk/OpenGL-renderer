#pragma once
#include "../gl/IndexBuffer.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"

#include <glm.hpp>

#include <memory>
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

  [[nodiscard]] GLsizei getIndexCount() const { return mIBO->getCount(); }

private:
  // Created in the constructor body: the index buffer must be bound while
  // this mesh's VAO is bound, otherwise it ends up in another VAO's state.
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::unique_ptr<IndexBuffer> mIBO;
};
