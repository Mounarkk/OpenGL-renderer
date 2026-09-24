#pragma once
#include "Model.h"

#include <memory>
#include <string>

/**
 * Imports model files with Assimp (OBJ, glTF/GLB, FBX, DAE...).
 *
 * The import generates what the renderer needs when the file lacks it
 * (triangles, smooth normals, tangents) and drops points and lines.
 * Textures are looked up relative to the model file. Embedded textures
 * (typical of .glb) are not supported yet and are replaced by defaults.
 */
class ModelLoader {
public:
  /// Throws a ResourceException if the file cannot be imported.
  static std::shared_ptr<Model> load(const std::string &path,
                                     const ModelImportOptions &options = {});
};
