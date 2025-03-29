#include "./application/Application.h"

int main() {
  Application app(800, 600, "Renderer");
  app.Run();
  return 0;
}