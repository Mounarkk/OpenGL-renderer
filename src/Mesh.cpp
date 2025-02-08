#include "Mesh.h"

#include "Cube.h"

Mesh::Mesh(std::vector<Vertex> &vertices,
           std::vector<unsigned int> &indices,
           std::vector<Texture_s> &textures) : mVBO(VertexBuffer(&mVertices[0], mVertices.size() * sizeof(Vertex))),
mIBO(IndexBuffer(&mIndices[0], mIndices.size() * sizeof(unsigned int))){
  this->mVertices = vertices;
  this->mIndices = indices;
  this->mTextures = textures;

  setupMesh();
}

void Mesh::setupMesh() {

  // Init the vertex array
   VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO = VertexArray();
  this->mVAO.addBuffer(this->mVBO, layout);
}

void Mesh::draw(const Shader &shader) {
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;

  for (unsigned int i = 0; i < mTextures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    // Retrieve texture number and set up uniforms
    std::string number;
    std::string name = mTextures[i].type;
    if (name == "texture_diffuse") {
      number = std::to_string(diffuseNr++);
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr++);
    }

    name.append(number);

    shader.setInt("material." + name, static_cast<int>(i));
    glBindTexture(GL_TEXTURE_2D, mTextures[i].id);
  }

  // Draw the mesh
  this->mVBO.bind();
  this->mIBO.bind();
  this->mVAO.bind();
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
}



