#include "ModelLoader.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "ResourceManager.h"

#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <limits>

namespace {
// No aiProcess_FlipUVs: Assimp already gives every format bottom-left UVs,
// which matches the vertically flipped images produced by Texture.
constexpr unsigned int kImportFlags =
    aiProcess_Triangulate | aiProcess_GenSmoothNormals |
    aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
    aiProcess_PreTransformVertices | aiProcess_SortByPType |
    aiProcess_ImproveCacheLocality;

constexpr float kMaxSmoothingAngle = 60.0f;

std::string directoryOf(const std::string &path) {
  const size_t lastSlash = path.find_last_of("/\\");
  return lastSlash == std::string::npos ? "" : path.substr(0, lastSlash + 1);
}

std::shared_ptr<Mesh> convertMesh(const aiMesh &mesh) {
  std::vector<Vertex> vertices;
  vertices.reserve(mesh.mNumVertices);

  for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
    Vertex vertex{};
    vertex.position = {mesh.mVertices[i].x, mesh.mVertices[i].y,
                       mesh.mVertices[i].z};
    if (mesh.HasNormals())
      vertex.normal = {mesh.mNormals[i].x, mesh.mNormals[i].y,
                       mesh.mNormals[i].z};
    if (mesh.HasTangentsAndBitangents()) {
      vertex.tangent = {mesh.mTangents[i].x, mesh.mTangents[i].y,
                        mesh.mTangents[i].z};
      vertex.bitangent = {mesh.mBitangents[i].x, mesh.mBitangents[i].y,
                          mesh.mBitangents[i].z};
    }
    if (mesh.HasTextureCoords(0))
      vertex.texCoords = {mesh.mTextureCoords[0][i].x,
                          mesh.mTextureCoords[0][i].y};
    vertices.push_back(vertex);
  }

  std::vector<unsigned int> indices;
  indices.reserve(static_cast<size_t>(mesh.mNumFaces) * 3);
  for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
    const aiFace &face = mesh.mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j)
      indices.push_back(face.mIndices[j]);
  }

  return std::make_shared<Mesh>(vertices, indices);
}
} // namespace

std::shared_ptr<Model> ModelLoader::load(const std::string &path,
                                         const ModelImportOptions &options) {
  Assimp::Importer importer;
  // Normals generated for files that lack them are only smoothed across
  // edges sharper than this angle, so boxes and walls keep hard edges.
  importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE,
                            kMaxSmoothingAngle);
  const unsigned int flags =
      kImportFlags | (options.flipUVs ? aiProcess_FlipUVs : 0u);
  const aiScene *scene = importer.ReadFile(path, flags);

  if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
      !scene->mRootNode)
    throw ResourceException(path, std::string("Assimp: ") +
                                      importer.GetErrorString());

  const std::string directory = directoryOf(path);

  // Materials are shared between the meshes that reference them
  std::vector<std::shared_ptr<Material>> materials(scene->mNumMaterials);
  for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    materials[i] =
        ResourceManager::loadMaterial(*scene->mMaterials[i], directory);

  auto model = std::make_shared<Model>();
  glm::vec3 boundsMin(std::numeric_limits<float>::max());
  glm::vec3 boundsMax(std::numeric_limits<float>::lowest());
  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    const aiMesh &mesh = *scene->mMeshes[i];
    if (!(mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
      continue; // Point clouds and lines left by SortByPType

    model->subMeshes.push_back({mesh.mName.C_Str(), convertMesh(mesh),
                                materials[mesh.mMaterialIndex]});

    for (unsigned int v = 0; v < mesh.mNumVertices; ++v) {
      const glm::vec3 p(mesh.mVertices[v].x, mesh.mVertices[v].y,
                        mesh.mVertices[v].z);
      boundsMin = glm::min(boundsMin, p);
      boundsMax = glm::max(boundsMax, p);
    }
  }
  if (!model->subMeshes.empty()) {
    model->boundsMin = boundsMin;
    model->boundsMax = boundsMax;
  }

  Logger::get()->info("Loaded model {} ({} meshes, {} materials)", path,
                      model->subMeshes.size(), materials.size());
  return model;
}
