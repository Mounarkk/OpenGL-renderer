#pragma once
#include "../rendering/Material.h"
#include "../rendering/Mesh.h"

#include <memory>
#include <string>
#include <vector>

/// Per-file import settings.
struct ModelImportOptions {
  /// Flips the V texture coordinate. Only needed for files authored with a
  /// top-left UV origin, like the LearnOpenGL backpack.
  bool flipUVs = false;
};

/// A mesh paired with the material it is drawn with.
struct SubMesh {
  std::string name;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

/**
 * GPU side data of an imported model file.
 *
 * The node hierarchy of the file is flattened at import time: node transforms
 * are baked into the vertices, so every sub-mesh is expressed in model space.
 * The same Model can be instantiated many times in a scene.
 */
struct Model {
  std::vector<SubMesh> subMeshes;

  /// Axis aligned bounds of all sub-meshes, in model space.
  glm::vec3 boundsMin{0.0f};
  glm::vec3 boundsMax{0.0f};
};
