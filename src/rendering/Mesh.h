#pragma once
#include "../gl/IndexBuffer.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"
#include "../gl/VertexBufferLayout.h"
#include "../rendering/Material.h"
#include <vector>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

class Mesh {
public:
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<unsigned int> &indices,
       const std::shared_ptr<Material> &material);
  ~Mesh();
  void draw() const;
  void clean();

private:
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::unique_ptr<IndexBuffer> mIBO;
  std::shared_ptr<Material> mMaterial;

  bool mDestroyed;
};