#include "Primitives.h"

#include <gtc/constants.hpp>

#include <cmath>
#include <vector>

namespace Primitives {

std::shared_ptr<Mesh> createSphere(const float radius, const int segments,
                                   const int rings) {
  std::vector<Vertex> vertices;
  vertices.reserve(static_cast<size_t>((segments + 1) * (rings + 1)));

  for (int ring = 0; ring <= rings; ++ring) {
    const float v = static_cast<float>(ring) / static_cast<float>(rings);
    const float phi = v * glm::pi<float>(); // 0 at the north pole
    for (int segment = 0; segment <= segments; ++segment) {
      const float u =
          static_cast<float>(segment) / static_cast<float>(segments);
      const float theta = u * glm::two_pi<float>();

      const glm::vec3 normal(std::sin(phi) * std::cos(theta), std::cos(phi),
                             std::sin(phi) * std::sin(theta));
      const glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));

      Vertex vertex{};
      vertex.position = normal * radius;
      vertex.normal = normal;
      vertex.tangent = tangent;
      vertex.bitangent = glm::cross(normal, tangent);
      vertex.texCoords = {u, 1.0f - v};
      vertices.push_back(vertex);
    }
  }

  // Counter-clockwise when seen from outside
  std::vector<unsigned int> indices;
  const auto stride = static_cast<unsigned int>(segments + 1);
  for (unsigned int ring = 0; ring < static_cast<unsigned int>(rings); ++ring) {
    for (unsigned int segment = 0;
         segment < static_cast<unsigned int>(segments); ++segment) {
      const unsigned int current = ring * stride + segment;
      const unsigned int below = current + stride;
      indices.insert(indices.end(), {current, current + 1, below});
      indices.insert(indices.end(), {current + 1, below + 1, below});
    }
  }

  return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> createPlane(const float halfSize, const float uvRepeat) {
  const glm::vec3 up(0.0f, 1.0f, 0.0f);
  const glm::vec3 tangent(1.0f, 0.0f, 0.0f);
  const glm::vec3 bitangent(0.0f, 0.0f, -1.0f); // V grows towards -Z

  const std::vector<Vertex> vertices = {
      {{-halfSize, 0.0f, halfSize}, up, tangent, bitangent, {0.0f, 0.0f}},
      {{halfSize, 0.0f, halfSize}, up, tangent, bitangent, {uvRepeat, 0.0f}},
      {{halfSize, 0.0f, -halfSize},
       up,
       tangent,
       bitangent,
       {uvRepeat, uvRepeat}},
      {{-halfSize, 0.0f, -halfSize}, up, tangent, bitangent, {0.0f, uvRepeat}}};
  const std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

  return std::make_shared<Mesh>(vertices, indices);
}

} // namespace Primitives
