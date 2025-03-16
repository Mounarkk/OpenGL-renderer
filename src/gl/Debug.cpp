#include "../core/Logger.h"
#include <glad/glad.h>

void EnableGLDebugging() {
  if (GLAD_GL_ARB_debug_output) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallbackARB(
        [](GLenum source, GLenum type, GLuint id, const GLenum severity,
           GLsizei length, const GLchar *message, const void *userParam) {
          if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
            Logger::get()->error("OpenGL Error: {}", message);
        },
        nullptr);
  } else {
    Logger::get()->warn("Debug output not supported!");
  }
}