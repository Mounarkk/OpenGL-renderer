#pragma once
#include "../rendering/Material.h"
#include "../scene/Entity.h"
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
  static void processNode(aiNode* node, const aiScene* aiScene, Scene& scene, Entity parent);
  static Entity processMesh(aiMesh* mesh, const aiScene* aiScene, Scene& scene, Entity parent);
};