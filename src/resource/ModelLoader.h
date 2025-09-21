#pragma once
#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "../scene/Components.h"
#include "../scene/Entity.h"
#include "../scene/Scene.h"
#include "ResourceManager.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <stb_image.h>
#include <string>
#include <vector>

/**
 * Static utility class for loading 3D models from files using Assimp.
 *
 * The ModelLoader class provides functionality to load complex 3D models
 * from various file formats (OBJ, FBX, GLTF, etc.) and convert them into
 * the engine's entity-component representation. It handles the complete
 * loading pipeline: parsing the file, extracting meshes and materials,
 * loading textures, and creating the appropriate entities and components
 * in the scene.
 *
 * Key features:
 * - Support for multiple 3D file formats via Assimp
 * - Automatic mesh and material extraction
 * - Texture loading and material setup
 * - Hierarchical scene graph preservation
 * - Entity-component integration
 * - Efficient resource sharing between meshes
 */
class ModelLoader {
public:
  /**
   * Loads a 3D model from file and creates entities in the specified scene.
   * @param scene Scene to create the model entities in
   * @param path File path to the 3D model (supports OBJ, FBX, GLTF, etc.)
   * @return Root entity of the loaded model hierarchy
   */
  static Entity load(Scene &scene, std::string &path);

private:
  /**
   * Recursively processes nodes in the Assimp scene graph.
   * Creates entities for each node and processes associated meshes.
   * @param node Current Assimp node to process
   * @param aiScene Complete Assimp scene data
   * @param scene Target scene for entity creation
   * @param parent Parent entity for hierarchical relationships
   * @param path Base path for resolving texture file paths
   */
  static void processNode(const aiNode *node, const aiScene *aiScene,
                          Scene &scene, Entity parent, const std::string &path);

  /**
   * Processes a single mesh from the Assimp scene.
   * Extracts vertex data, creates materials, and sets up the mesh entity.
   * @param mesh Assimp mesh data to process
   * @param aiScene Complete Assimp scene for material access
   * @param scene Target scene for entity creation
   * @param parent Parent entity for the mesh
   * @param path Base path for resolving texture file paths
   * @return Entity representing the processed mesh
   */
  static Entity processMesh(const aiMesh *mesh, const aiScene *aiScene,
                            Scene &scene, Entity parent,
                            const std::string &path);
};