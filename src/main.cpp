#include "./application/Application.h"

int main() {
  // Use configuration-based constructor for better flexibility
  Application app("OpenGL Renderer");
  app.Run();
  return 0;
}