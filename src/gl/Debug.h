#pragma once
#include <glad/glad.h>

/// Routes driver debug messages (core since OpenGL 4.3) to the logger.
void enableGLDebugging();

/**
 * Names a section of the frame in graphics debuggers such as RenderDoc or
 * Nsight, for as long as the object lives.
 *
 * ```cpp
 * GLDebugGroup group("Shadow pass");
 * ```
 */
class GLDebugGroup {
public:
  explicit GLDebugGroup(const char *name);
  ~GLDebugGroup();

  GLDebugGroup(const GLDebugGroup &) = delete;
  GLDebugGroup &operator=(const GLDebugGroup &) = delete;
};
