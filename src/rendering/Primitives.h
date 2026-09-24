#pragma once
#include "Mesh.h"

#include <memory>

/// Procedural meshes, built with the regular vertex format (normals,
/// tangents and UVs included) so any material can be used on them.
namespace Primitives {

/// UV sphere centered on the origin.
/// @param segments Subdivisions around the vertical axis
/// @param rings Subdivisions from pole to pole
std::shared_ptr<Mesh> createSphere(float radius, int segments = 24,
                                   int rings = 16);

/// Horizontal square facing +Y, centered on the origin.
/// @param uvRepeat Number of times the texture repeats along each side
std::shared_ptr<Mesh> createPlane(float halfSize, float uvRepeat);

} // namespace Primitives
