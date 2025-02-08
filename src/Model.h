#ifndef MODEL_H
#define MODEL_H
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

class Model {
public:
  explicit Model(const std::string &path) {
    loadModel(path);
  }
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
  std::vector<Texture_s> loadMaterialTextures(const aiMaterial *mat, aiTextureType type,
                                              const std::string &typeName);
};



#endif //MODEL_H
