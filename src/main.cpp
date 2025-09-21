#include "./application/Application.h"
#include "./core/RendererException.h"
#include "./core/Logger.h"

int main() {
  try {
    // Use configuration-based constructor for better flexibility
    Application app("OpenGL Renderer");
    app.Run();
    return 0;
  } catch (const RendererException& e) {
    // Handle renderer-specific exceptions with detailed error information
    if (Logger::get()) {
      Logger::get()->critical("Renderer initialization failed: {}", e.what());
    } else {
      // Fallback if logger isn't initialized yet
      std::cerr << "Critical Error: " << e.what() << std::endl;
    }
    return -1;
  } catch (const std::exception& e) {
    // Handle any other standard exceptions
    if (Logger::get()) {
      Logger::get()->critical("Unexpected error: {}", e.what());
    } else {
      std::cerr << "Critical Error: " << e.what() << std::endl;
    }
    return -1;
  } catch (...) {
    // Handle any unknown exceptions
    if (Logger::get()) {
      Logger::get()->critical("Unknown critical error occurred");
    } else {
      std::cerr << "Critical Error: Unknown error occurred" << std::endl;
    }
    return -1;
  }
}