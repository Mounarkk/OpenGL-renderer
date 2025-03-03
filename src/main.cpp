#include "../vendor/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "../vendor/stb_image/stb_image.h"
#include "Camera.h"
#include "Cube.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "core/Logger.h"

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

  // build and compile textureShader programs
  // ------------------------------------
  const Shader lightShader("../res/shaders/colored_cube_shader.vert",
                           "../res/shaders/light_cube_shader.frag");
  const Shader textureShader("../res/shaders/textured_cube_shader.vert",
                             "../res/shaders/textured_cube_shader.frag");

  Model backPack("../res/models/backpack/backpack.obj");


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

      textureShader.use();

      textureShader.setVec3("viewPos", camera.mPosition.x, camera.mPosition.y,
                            camera.mPosition.z);

      // pass projection matrix to textureShader (note that in this case it could change
      // every frame)
      const glm::mat4 projection = glm::perspective(
          glm::radians(camera.mZoom),
          static_cast<float>(800) / static_cast<float>(600), 0.1f, 100.0f);
      textureShader.setMat4("projection", projection);

      // camera/view transformation
      const glm::mat4 view = camera.getViewMatrix();
      textureShader.setMat4("view", view);

      // calculate the model matrix
      auto model = glm::mat4(
          1.0f); // make sure to initialize matrix to identity matrix first
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
      textureShader.setMat4("model", model);

      textureShader.setFloat("material.shininess", 64.0f);

      // Directional light parameters
      textureShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
      textureShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
      textureShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
      textureShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

      // Point lights parameters
      constexpr glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
      };

      for (int i = 0; i < 4; i++) {
        std::ostringstream oss;
        oss << "pointLights[" << i << "]";
        textureShader.setVec3(oss.str() + ".position", pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
        textureShader.setVec3(oss.str() + ".ambient", 0.02f, 0.02f, 0.02f);
        textureShader.setVec3(oss.str() + ".diffuse", 0.5f, 0.5f, 0.5f);
        textureShader.setVec3(oss.str() + ".specular", 1.0f, 1.0f, 1.0f);
        textureShader.setFloat(oss.str() + ".constant", 1.0f);
        textureShader.setFloat(oss.str() + ".linear", 0.09f);
        textureShader.setFloat(oss.str() + ".quadratic", 0.032);
      }

      // Spotlight parameters
      textureShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
      textureShader.setVec3("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
      textureShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
      textureShader.setVec3("spotLight.position", camera.mPosition.x, camera.mPosition.y, camera.mPosition.z);
      textureShader.setVec3("spotLight.direction", camera.mFront.x, camera.mFront.y, camera.mFront.z);
      textureShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
      textureShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

      GLenum err;
      while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error BEFORE drawing: " << err << std::endl;
      }

      backPack.draw(textureShader);
      GLenum err_a;
      while ((err_a = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error AFTER drawing: " << err_a << std::endl;
      }

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
