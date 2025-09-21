#pragma once
#include "RenderPass.h"
#include "../core/Config.h"

/**
 * Core rendering system that manages render commands and OpenGL state.
 * 
 * The Renderer class collects rendering commands during a frame and executes
 * them efficiently. It manages the command queue, render passes, and coordinates
 * with OpenGL to draw geometry. The renderer uses a command-based approach
 * where objects submit render commands that are batched and executed together.
 * 
 * Key responsibilities:
 * - Command queue management and batching
 * - OpenGL state management and optimization
 * - Render pass coordination and execution
 * - Viewport and framebuffer management
 */
class Renderer {
public:
  /**
   * Constructs the renderer and initializes render passes.
   */
  Renderer();
  
  /**
   * Destructor that cleans up render passes and OpenGL resources.
   */
  ~Renderer();
  
  /**
   * Clears the screen and prepares for a new frame.
   * Sets up the default framebuffer and clears color/depth buffers.
   */
  void clear();
  
  /**
   * Submits a render command to be executed later.
   * Commands are queued and executed together during flush() for efficiency.
   * @param command The render command containing mesh, material, and transform data
   */
  void submit(const RenderCommand &command);
  
  /**
   * Executes all queued render commands with the given view-projection matrix.
   * This processes the command queue, sets up shaders, and draws all geometry.
   * @param viewProj Combined view and projection matrix for the current camera
   */
  void flush(const glm::mat4 &viewProj);
  
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
private:
  std::vector<RenderCommand> m_CommandQueue;
  std::vector<RenderPass*> mRenderPasses;
};