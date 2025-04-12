#pragma once
#include "../rendering/Mesh.h"
#include "GLFW/glfw3.h"

class GLObjectDestroyer {
public:
  static GLObjectDestroyer& getInstance();

  void registerMesh(const std::shared_ptr<Mesh>& mesh);

  void cleanupAll();

private:
  GLObjectDestroyer() = default;
  ~GLObjectDestroyer() = default;

  // No copying or assignment
  GLObjectDestroyer(const GLObjectDestroyer&) = delete;
  GLObjectDestroyer& operator=(const GLObjectDestroyer&) = delete;

  std::vector<std::shared_ptr<Mesh>> mMeshes;
};
