#include "Debug.h"
#include "../core/Logger.h"

#include <glad/glad.h>

void enableGLDebugging() {
  if (!GLAD_GL_ARB_debug_output) {
    Logger::get()->warn("GL_ARB_debug_output not supported, GL errors will "
                        "not be reported");
    return;
  }

  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
  glDebugMessageCallbackARB(
      [](GLenum, GLenum, GLuint, const GLenum severity, GLsizei,
         const GLchar *message, const void *) {
        if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
          Logger::get()->error("OpenGL: {}", message);
        else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
          Logger::get()->warn("OpenGL: {}", message);
      },
      nullptr);
}
