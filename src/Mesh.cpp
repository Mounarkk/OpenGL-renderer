#include "Mesh.h"

#include "Cube.h"

Mesh::Mesh(std::vector<Vertex> &vertices,
           std::vector<unsigned int> &indices,
           std::vector<Texture_s> &textures) {
  this->mVertices = vertices;
  this->mIndices = indices;
  this->mTextures = textures;

  setupMesh();
}

void Mesh::setupMesh() {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);

  // Vérification après allocation des buffers
  this->mIBO = new IndexBuffer(&mIndices[0], mIndices.size() * sizeof(unsigned int));
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) std::cout << "OpenGL error after IBO creation: " << err << std::endl;

  this->mVBO = new VertexBuffer(&mVertices[0], mVertices.size() * sizeof(Vertex));
  err = glGetError();
  if (err != GL_NO_ERROR) std::cout << "OpenGL error after VBO creation: " << err << std::endl;

  this->mVAO = new VertexArray();
  err = glGetError();
  if (err != GL_NO_ERROR) std::cout << "OpenGL error after VAO creation: " << err << std::endl;

  this->mVAO->addBuffer(*this->mVBO, layout);
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
      number = std::to_string(diffuseNr);
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr);
    }

    name.append(number);

    shader.setInt("material." + name, static_cast<int>(i));
    glBindTexture(GL_TEXTURE_2D, mTextures[i].id);
  }


  // Draw the mesh
  this->mVAO->bind();
  this->mVBO->bind();
  this->mIBO->bind();
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
  this->mVAO->unbind();
}



