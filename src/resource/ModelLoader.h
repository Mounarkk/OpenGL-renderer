#pragma once
#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "../scene/Entity.h"
#include "../scene/Components.h"
#include "ResourceManager.h"
#include <stb_image.h>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <string>
#include <vector>

class ModelLoader {
public:
  static Entity load(Scene& scene, const std::string& path);

private:
  static void processNode(const aiNode * node, const aiScene* aiScene, Scene& scene, Entity parent);
  static Entity processMesh(const aiMesh * mesh, const aiScene* aiScene, Scene& scene, Entity parent);
};