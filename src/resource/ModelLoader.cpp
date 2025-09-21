#include "ModelLoader.h"
#include "../core/RendererException.h"

#include "../gl/GLObjectDestroyer.h"

Entity ModelLoader::load(Scene &scene, std::string &path) {
  Assimp::Importer importer;
  const aiScene *aiScene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                  aiProcess_CalcTangentSpace);

  if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !aiScene->mRootNode) {
    Logger::get()->error("Failed to load model: {}", importer.GetErrorString());
    throw ResourceException(path, "Assimp loading failed: " +
                                      std::string(importer.GetErrorString()));
  }

  // Get the object path
  if (const size_t last_slash = path.find_last_of('/');
      last_slash != std::string::npos) {
    path = path.substr(0, last_slash + 1);
  }

  const Entity root = scene.createEntity("ModelRoot");
  processNode(aiScene->mRootNode, aiScene, scene, root, path);
  return root;
}

void ModelLoader::processNode(const aiNode *node, const aiScene *aiScene,
                              Scene &scene, Entity parent,
                              const std::string &path) {
  // Create an entity for this node
  Entity entity = scene.createEntity(node->mName.C_Str());
  // Add default transform component
  entity.addComponent<Transform>();

  // Process meshes
  for (unsigned i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = aiScene->mMeshes[node->mMeshes[i]];
    // Process mesh for this node
    processMesh(mesh, aiScene, scene, entity, path);
  }

  // Process children
  for (unsigned i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], aiScene, scene, entity, path);
  }
}

Entity ModelLoader::processMesh(const aiMesh *mesh, const aiScene *aiScene,
                                Scene &scene, Entity parent,
                                const std::string &path) {
  // Convert Assimp mesh to our Mesh class
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // Process vertices
  for (unsigned i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex{};
    vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                       mesh->mVertices[i].z};
    if (mesh->HasNormals()) {
      vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                       mesh->mNormals[i].z};
    }
    if (mesh->HasTangentsAndBitangents()) {
      vertex.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y,
                        mesh->mTangents[i].z};
      vertex.bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                          mesh->mBitangents[i].z};
    }
    if (mesh->mTextureCoords[0]) {
      vertex.texCoords = {mesh->mTextureCoords[0][i].x,
                          mesh->mTextureCoords[0][i].y};
    } else {
      vertex.texCoords = {0.0f, 0.0f};
    }
    vertices.push_back(vertex);
  }

  // Process indices
  for (unsigned i = 0; i < mesh->mNumFaces; i++) {
    const aiFace face = mesh->mFaces[i];
    for (unsigned j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Load material
  const aiMaterial *aiMaterial = aiScene->mMaterials[mesh->mMaterialIndex];
  auto material = ResourceManager::loadMaterial(aiMaterial, path);

  // Create a Mesh and assign it to an entity
  auto myMesh = std::make_shared<Mesh>(vertices, indices, material);
  GLObjectDestroyer::getInstance().registerMesh(myMesh);

  Entity meshEntity = scene.createEntity(mesh->mName.C_Str());
  // Add default transform component
  meshEntity.addComponent<Transform>();
  meshEntity.addComponent<MeshRenderer>(myMesh, material);
  return meshEntity;
}