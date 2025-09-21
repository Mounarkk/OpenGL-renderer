#pragma once
#include "../core/Config.h"
#include "../scene/Scene.h"
#include "RenderPass.h"
#include <memory>

/**
 * Core rendering system that manages render commands and OpenGL state.
 *
 * The Renderer class collects rendering commands during a frame and executes
 * them efficiently. It manages the command queue, render passes, and
 * coordinates with OpenGL to draw geometry. The renderer uses a command-based
 * approach where objects submit render commands that are batched and executed
 * together.
 *
 * Key responsibilities:
 * - Command queue management and batching
 * - OpenGL state management and optimization
 * - Render pass coordination and execution
 * - Viewport and framebuffer management
 *
 * Ownership Patterns:
 * - Owns render passes via std::unique_ptr (exclusive ownership)
 * - Command queue is owned by the renderer instance
 * - Render commands contain shared_ptr to meshes and materials (shared
 * ownership)
 */
class Renderer {
public:
  /**
   * Constructs the renderer and initializes render passes.
   */
  Renderer();

  /**
   * Virtual destructor for proper inheritance cleanup.
   */
  virtual ~Renderer() = default;

  /**
   * Static method to clear the screen and prepare for a new frame.
   * Sets up the default framebuffer and clears color/depth buffers.
   */
  static void clear();

  /**
   * Prepares the command queue by processing scene objects.
   * Collects render commands from scene entities for later execution.
   * @param scene The scene containing all renderable objects
   */
  void prepareCommandQueue(Scene &scene);

  /**
   * Executes all queued render commands with the given matrices.
   * This processes the command queue, sets up shaders, and draws all geometry.
   * @param projMat Projection matrix for the current camera
   * @param viewMat View matrix for the current camera
   */
  void flush(const glm::mat4 &projMat, const glm::mat4 &viewMat);

  /**
   * Cleans up all renderer resources.
   * Should be called before application shutdown.
   */
  void cleanup() const;

  /**
   * Updates the renderer for a new viewport size.
   * Adjusts internal framebuffers and OpenGL viewport settings.
   * @param width New viewport width in pixels
   * @param height New viewport height in pixels
   */
  void resize(int width, int height) const;

protected:
  std::vector<RenderCommand> s_CommandQueue;
  std::vector<std::unique_ptr<RenderPass>> mRenderPasses;
};

class ForwardRenderer final : public Renderer {
public:
  ForwardRenderer();
  ~ForwardRenderer() override;
};