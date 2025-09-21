#pragma once
#include "../gl/IndexBuffer.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"
#include "../gl/VertexBufferLayout.h"
#include "../rendering/Material.h"
#include <vector>

/**
 * Vertex data structure containing all attributes needed for rendering.
 * This structure defines the layout of vertex data in memory and includes
 * all information needed for modern PBR rendering techniques.
 */
struct Vertex {
  glm::vec3 position;  // 3D position in model space
  glm::vec3 normal;    // Surface normal vector
  glm::vec3 tangent;   // Tangent vector for normal mapping
  glm::vec3 bitangent; // Bitangent vector for normal mapping
  glm::vec2 texCoords; // UV texture coordinates
};

/**
 * Renderable mesh containing geometry data and rendering state.
 *
 * The Mesh class represents a piece of 3D geometry that can be rendered.
 * It encapsulates vertex data, index data, and the associated OpenGL objects
 * needed for efficient rendering. Each mesh can have an associated material
 * that defines its surface properties. The class handles the complete setup
 * of vertex attributes and provides a simple draw interface.
 *
 * Key features:
 * - Efficient vertex and index buffer management
 * - Automatic vertex attribute layout setup
 * - Material association for surface properties
 * - Optimized rendering with indexed geometry
 * - Proper resource cleanup and state management
 */
class Mesh {
public:
  /**
   * Constructs a mesh from vertex and index data with an associated material.
   * Sets up all OpenGL objects (VAO, VBO, IBO) and configures vertex
   * attributes.
   * @param vertices Vector of vertex data containing positions, normals, UVs,
   * etc.
   * @param indices Vector of indices for indexed rendering (triangles)
   * @param material Shared pointer to material defining surface properties
   */
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<unsigned int> &indices,
       const std::shared_ptr<Material> &material);

  /**
   * Destructor that automatically cleans up all OpenGL resources.
   */
  ~Mesh();

  /**
   * Renders this mesh using its associated material.
   * Binds the VAO and issues the draw call with indexed rendering.
   */
  void draw() const;

  /**
   * Manually cleans up all OpenGL resources.
   * Called automatically by destructor.
   */
  void clean();

private:
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::unique_ptr<IndexBuffer> mIBO;
  std::shared_ptr<Material> mMaterial;

  bool mDestroyed;
};