#include "GLObjectDestroyer.h"

GLObjectDestroyer& GLObjectDestroyer::getInstance() {
  static GLObjectDestroyer instance; // Thread-safe, one-time init
  return instance;
}

void GLObjectDestroyer::registerMesh(const std::shared_ptr<Mesh>& mesh) {
  mMeshes.push_back(mesh);
}

void GLObjectDestroyer::cleanupAll() {
  for (auto& mesh : mMeshes) {
    if (mesh) {
      mesh->clean(); // Call your custom mesh GL cleanup logic
    }
  }
  mMeshes.clear(); // Drop all shared_ptrs
}


