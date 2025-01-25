#include "../vendor/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../vendor/stb_image/stb_image.h"
#include "Camera.h"
#include "Cube.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <iostream>

void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xPosIn, double yPosIn);
void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void processInput(GLFWwindow *window);

// settings
constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

int main() {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetScrollCallback(window, scrollCallback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile shader programs
  // ------------------------------------
  const Shader lightShader("../res/shaders/colored_cube_shader.vert",
                           "../res/shaders/light_cube_shader.frag");
  const Shader textureShader("../res/shaders/textured_cube_shader.vert",
                           "../res/shaders/textured_cube_shader.frag");

  // add a scope here to destroy the vertex buffers/array before terminating the
  // opengl context
  {
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
      // per-frame time logic
      // --------------------
      const auto currentFrame = static_cast<float>(glfwGetTime());
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      ColoredCube lightCube(glm::vec3(2.0f, 1.0f, -5.7f),
                            glm::vec3(1.0f, 1.0f, 1.0f));
      TexturedCube texturedCube(glm::vec3(0.0f, 0.0f, 0.0f), "../res/textures/container2.png", GL_RGBA, "../res/textures/container2_specular.png", GL_RGBA);

      textureShader.setVec3("lightPos", 0.0f, 1.0f, -4.7f);
      textureShader.setVec3("viewPos", camera.mPosition.x, camera.mPosition.y,
                          camera.mPosition.z);

      lightCube.draw(lightShader, camera);
      texturedCube.draw(textureShader, camera);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse
      // moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // NOTE : objects will be destroyed and call their respective class
    // destructor
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void frameBufferSizeCallback(GLFWwindow *window, const int width,
                             const int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouseCallback(GLFWwindow *window, const double xPosIn,
                   const double yPosIn) {
  const auto xPos = static_cast<float>(xPosIn);
  const auto yPos = static_cast<float>(yPosIn);

  if (firstMouse) // prevent sudden jumps when entering the created window
  {
    lastX = xPos;
    lastY = yPos;
    firstMouse = false;
  }

  const float xOffset = xPos - lastX;
  const float yOffset =
      lastY - yPos; // reversed since y-coordinates go from bottom to top

  lastX = xPos;
  lastY = yPos;

  camera.processMouseMovement(xOffset, yOffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scrollCallback(GLFWwindow *window, const double xOffset,
                    const double yOffset) {
  camera.processMouseScroll(static_cast<float>(yOffset));
}
