#ifndef MESH_H
#define MESH_H
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"

#include <vec2.hpp>
#include <vec3.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

struct Texture_s {
  unsigned int id;
  std::string type;
  std::string path; // store the path to check if the texture has already been generated
};

class Mesh {
public:
  std::vector<Vertex> mVertices;
  std::vector<unsigned int> mIndices;
  std::vector<Texture_s> mTextures;

  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture_s> &textures);
  void draw(const Shader &shader);

private:
  VertexArray mVAO;
  VertexBuffer mVBO;
  IndexBuffer mIBO;

  void setupMesh();
};



#endif //MESH_H
