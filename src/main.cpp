#include "application/Application.h"
#include "core/Logger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
void printUsage(const char *program) {
  std::cout << "Usage: " << program << " [options]\n"
            << "  --model <file>        display a model instead of the test "
               "scene\n"
            << "  --scale <factor>      uniform scale applied to --model\n"
            << "  --flip-uvs            for --model files with top-left UVs\n"
            << "  --camera x,y,z,yaw,pitch  initial camera, angles in degrees\n"
            << "  --cascades            start with the cascade debug view\n"
            << "  --screenshot <file>   render a few frames, save a PNG and "
               "quit\n";
}
} // namespace

int main(int argc, char **argv) {
  ApplicationOptions options;
  for (int i = 1; i < argc; ++i) {
    const bool hasValue = i + 1 < argc;
    if (std::strcmp(argv[i], "--screenshot") == 0 && hasValue) {
      options.screenshotPath = argv[++i];
    } else if (std::strcmp(argv[i], "--model") == 0 && hasValue) {
      options.modelPath = argv[++i];
    } else if (std::strcmp(argv[i], "--scale") == 0 && hasValue) {
      options.modelScale = std::strtof(argv[++i], nullptr);
    } else if (std::strcmp(argv[i], "--camera") == 0 && hasValue) {
      float x, y, z, yaw, pitch;
      if (std::sscanf(argv[++i], "%f,%f,%f,%f,%f", &x, &y, &z, &yaw, &pitch) !=
          5) {
        printUsage(argv[0]);
        return 1;
      }
      options.hasCameraOverride = true;
      options.cameraPosition = {x, y, z};
      options.cameraYaw = yaw;
      options.cameraPitch = pitch;
    } else if (std::strcmp(argv[i], "--flip-uvs") == 0) {
      options.modelImport.flipUVs = true;
    } else if (std::strcmp(argv[i], "--cascades") == 0) {
      options.showCascades = true;
    } else {
      printUsage(argv[0]);
      return 1;
    }
  }

  try {
    Application app("OpenGL Renderer", options);
    app.Run();
  } catch (const std::exception &e) {
    if (Logger::get())
      Logger::get()->critical("{}", e.what());
    else
      std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
