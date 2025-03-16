#ifndef MODEL_H
#define MODEL_H
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma = false);

class Model {
public:
  explicit Model(const std::string &path) { loadModel(path); }
  void draw(const Shader &shader);

private:
  // optimisation
  std::vector<Texture_s> mTexturesLoaded;
  // model data
  std::vector<Mesh> mMeshes;
  std::string mDirectory;

  void loadModel(const std::string &path);
  void processNode(const aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture_s> loadMaterialTextures(aiMaterial *mat,
                                              aiTextureType type,
                                              std::string typeName);
};

#endif // MODEL_H
