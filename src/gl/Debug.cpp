#include "Debug.h"
#include "../core/Logger.h"

void enableGLDebugging() {
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(
      [](GLenum, GLenum, GLuint, const GLenum severity, GLsizei,
         const GLchar *message, const void *) {
        if (severity == GL_DEBUG_SEVERITY_HIGH)
          Logger::get()->error("OpenGL: {}", message);
        else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
          Logger::get()->warn("OpenGL: {}", message);
      },
      nullptr);
}

GLDebugGroup::GLDebugGroup(const char *name) {
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
}

GLDebugGroup::~GLDebugGroup() { glPopDebugGroup(); }
