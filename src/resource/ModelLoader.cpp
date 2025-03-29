#include "ModelLoader.h"

Entity ModelLoader::load(Scene &scene, const std::string &path) {
  Assimp::Importer importer;
  const aiScene *aiScene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !aiScene->mRootNode) {
    Logger::get()->error("Failed to load model: {}", importer.GetErrorString());
    return Entity{entt::null, nullptr}; // Invalid entity
  }

  const Entity root = scene.createEntity("ModelRoot");
  processNode(aiScene->mRootNode, aiScene, scene, root);
  return root;
}

void ModelLoader::processNode(const aiNode *node, const aiScene *aiScene,
                              Scene &scene, Entity parent) {
  // Create an entity for this node
  Entity entity = scene.createEntity(node->mName.C_Str());
  // TODO: Default transform given here
  entity.addComponent<Transform>();

  // Process meshes
  for (unsigned i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = aiScene->mMeshes[node->mMeshes[i]];
    // TODO: Maybe make a parent-child relationship system via Entt ?
    processMesh(mesh, aiScene, scene, entity);
  }

  // Process children
  for (unsigned i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], aiScene, scene, entity);
  }
}

Entity ModelLoader::processMesh(const aiMesh *mesh, const aiScene *aiScene,
                                Scene &scene, Entity parent) {
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
  auto material = ResourceManager::loadMaterial(aiMaterial);

  // Create a Mesh and assign it to an entity
  auto myMesh = std::make_shared<Mesh>(vertices, indices, material);

  Entity meshEntity = scene.createEntity(mesh->mName.C_Str());
  // TODO: Default transform given here
  meshEntity.addComponent<Transform>();
  meshEntity.addComponent<MeshRenderer>(myMesh, material);
  return meshEntity;
}