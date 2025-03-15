#include "ModelLoader.h"
#include "ResourceManager.h"

Entity ModelLoader::load(Scene& scene, const std::string& path) {
    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode) {
        Logger::get()->error("Failed to load model: {}", importer.GetErrorString());
        return Entity(); // Invalid entity
    }

    Entity root = scene.createEntity("ModelRoot");
    processNode(aiScene->mRootNode, aiScene, scene, root);
    return root;
}

void ModelLoader::processNode(aiNode* node, const aiScene* aiScene, Scene& scene, Entity parent) {
    // Create an entity for this node
    Entity entity = scene.CreateEntity(node->mName.C_Str());
    entity.AddComponent<Transform>().SetParent(parent);

    // Process meshes
    for (unsigned i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = aiScene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, aiScene, scene, entity);
    }

    // Process children
    for (unsigned i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], aiScene, scene, entity);
    }
}

Entity ModelLoader::processMesh(aiMesh* mesh, const aiScene* aiScene, Scene& scene, Entity parent) {
    // Convert Assimp mesh to our Mesh class
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    // Process vertices
    for (unsigned i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 vertex;
        vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        if (mesh->HasNormals()) {
            vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        } else {
            vertex.texCoords = {0.0f, 0.0f};
        }
        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Load material
    aiMaterial* aiMaterial = aiScene->mMaterials[mesh->mMaterialIndex];
    auto material = ResourceManager::LoadMaterial(aiMaterial);

    // Create a Mesh and assign it to an entity
    auto myMesh = std::make_shared<Mesh>(vertices, indices, material);

    Entity meshEntity = scene.CreateEntity("Mesh");
    meshEntity.AddComponent<Transform>().SetParent(parent);
    meshEntity.AddComponent<MeshRenderer>(myMesh, material);
    return meshEntity;
}