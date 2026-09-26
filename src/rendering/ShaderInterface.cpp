#include "ShaderInterface.h"

#include <sstream>

namespace ShaderInterface {

std::string defines() {
  std::ostringstream out;
  out << "#define FRAME_BLOCK " << kFrameBlock << "\n"
      << "#define LIGHTS_BLOCK " << kLightsBlock << "\n"
      << "#define SHADOW_BLOCK " << kShadowBlock << "\n"
      << "#define SHADOW_MAP_UNIT " << kShadowMapUnit << "\n"
      << "#define ALBEDO_UNIT " << kAlbedoUnit << "\n"
      << "#define SPECULAR_UNIT " << kSpecularUnit << "\n"
      << "#define NORMAL_UNIT " << kNormalUnit << "\n"
      << "#define SKYBOX_UNIT " << kSkyboxUnit << "\n"
      << "#define SCREEN_UNIT " << kScreenUnit << "\n"
      << "#define CASCADE_COUNT " << kCascadeCount << "\n"
      << "#define MAX_POINT_LIGHTS " << kMaxPointLights << "\n"
      << "#define MAX_SPOT_LIGHTS " << kMaxSpotLights << "\n";
  return out.str();
}

} // namespace ShaderInterface
