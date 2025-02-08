#include "Model.h"

void Model::draw(const Shader &shader) {
  for (auto & mesh : mMeshes) {
    mesh.draw(shader);
  }
}

void Model::loadModel(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }

  this->mDirectory = path.substr(0, path.find_last_of('/'));

  processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode *node, const aiScene *scene) {
  // process all the node's meshes
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    this->mMeshes.push_back(processMesh(mesh, scene));
  }

  if (node->mNumChildren == 0) {
    return;
  }

  // Recursively iterate on the node's children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  // Initialize the data to convert assimp mesh into application mesh
  std::vector<Vertex> vertices(mesh->mNumVertices);
  std::vector<unsigned int> indices(mesh->mNumFaces * 3);
  std::vector<Texture_s> textures(mesh->mNumFaces);

  // process vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex{};
    glm::vec3 position;
    position.x = mesh->mVertices[i].x;
    position.y = mesh->mVertices[i].y;
    position.z = mesh->mVertices[i].z;
    vertex.position = position;

    glm::vec3 normal;
    normal.x = mesh->mNormals[i].x;
    normal.y = mesh->mNormals[i].y;
    normal.z = mesh->mNormals[i].z;
    vertex.normal = normal;

    if (mesh->mTextureCoords[0]) {
      glm::vec2 texCoords;
      texCoords.x = mesh->mTextureCoords[0][0].x;
      texCoords.y = mesh->mTextureCoords[0][0].y;
      vertex.texCoords = texCoords;
    } else {
      vertex.texCoords = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  // process indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  std::vector<Texture_s> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  std::vector<Texture_s> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

  return Mesh(vertices, indices, textures);
}

std::vector<Texture_s> Model::loadMaterialTextures(const aiMaterial *mat,
                                                   const aiTextureType type,
                            const std::string &typeName) {
  std::vector<Texture_s> textures;

  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    std::string filePath = mDirectory + '/' + str.C_Str(); // string got from the material is local to the directory, hence the concatenation
    // Checks if the texture has already been loaded
    bool skip = false;
    for (unsigned int j = 0; j < mTexturesLoaded.size(); j++) {
      if (mTexturesLoaded[i].path == filePath) {
        skip = true;
        break;
      }
    }

    if (!skip) {
      Texture texture(filePath);

      Texture_s texture_s;
      texture_s.id = texture.getRendererId();
      texture_s.type = typeName;
      texture_s.path = filePath;
      textures.push_back(texture_s);
      this->mTexturesLoaded.push_back(texture_s);
    }
  }

  return textures;
}