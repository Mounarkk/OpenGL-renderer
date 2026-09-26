# Cascaded shadow maps

Notes on how the directional light shadows are implemented, and why.

Code: `ShadowMappingPass` in `src/rendering/RenderPass.cpp`, the
`shadowm_shader.*` shaders and `sunShadow` in `forward_shader.frag`. The
cascade data reaches the lighting shader through the shadow uniform block
(`res/shaders/common/shadows.glsl`).

## Splitting the frustum

A single shadow map stretched over the whole view wastes most of its texels
far away and lacks them up close. The camera frustum is therefore cut into
4 slices along its depth, and each slice gets its own shadow map of
`shadow.map_size` texels (a layer of a `GL_TEXTURE_2D_ARRAY`).

Split distances blend a logarithmic and a uniform distribution (the
"practical split scheme" from GPU Gems 3, chapter 10), with a weight of
0.8 on the logarithmic part:

```
split_i = 0.8 * n * (f / n)^(i / N) + 0.2 * (n + (f - n) * i / N)
```

Pure logarithmic splits make the first cascade tiny, pure uniform splits give
the close range far too few texels.

## Fitting a cascade

For each slice, the 8 corners of the sub-frustum are computed in world space
by unprojecting the NDC cube with the inverse of the slice's view-projection.

The light projection is an orthographic box around the **bounding sphere** of
those corners rather than their bounding box. A box changes size whenever the
camera rotates, which rescales the shadow map texels and makes shadow edges
swim. A sphere does not depend on the orientation.

The projection is then **snapped to the texel grid**: the world origin is
projected into shadow map texel space, rounded, and the difference is added
to the projection's translation. Moving the camera then only shifts the
shadow map by whole texels, so edges stay still.

Along the light direction, the box extends 4 radii towards the light so that
occluders outside the view (a building behind the camera) still cast shadows.
`GL_DEPTH_CLAMP` is enabled during the pass, so anything even closer to the
light is flattened onto the near plane instead of being clipped.

## Rendering all cascades in one pass

The framebuffer has the whole texture array attached, which makes it a
layered framebuffer. The vertex shader only applies the model matrix. The
geometry shader is instanced (`invocations = CASCADE_COUNT`): each invocation
handles one cascade and writes the triangle to its layer through `gl_Layer`.
The scene is submitted once instead of once per cascade.

Each mesh is tested on the CPU against the frustum of every cascade, and the
result is passed as a bit mask. Invocations whose bit is not set return
immediately, and meshes outside every cascade are not drawn at all. The near
plane is ignored by this test, since casters in front of it are clamped onto
it rather than clipped.

The shadow fragment shader samples the albedo alpha and discards cut-out
texels, so foliage and fences cast the right shape.

## Sampling

In the lighting shader:

1. The cascade is picked from the view-space depth of the fragment.
2. The world position is pushed along the normal by about one texel of that
   cascade (more at grazing angles) before being projected. This
   **normal offset** removes most self-shadowing acne.
3. The remaining depth bias is expressed in texels: a constant part plus a
   part proportional to the tangent of the angle between the surface and the
   light, clamped. Since each cascade knows the depth span of one of its
   texels, the same constants work for every cascade.
4. A 3x3 PCF kernel softens the edges.

Press `C` to tint each cascade with a different color.

## Checking the shadows

The sun is drawn in the sky at the exact direction of the light, and its
angles are shown in the window title. Shadows must point away from the disc.

- `L` turns off the point and spot lights, so only the sun lights the scene.
- The arrow keys move the sun: `Left`/`Right` for azimuth, `Up`/`Down` for
  elevation. Moving it low makes long shadows that are easy to follow.
- The same setup from the command line, with the sun in front of the camera:

```bash
./build/opengl_renderer --sun 270,12 --sun-only --camera 1.5,1.6,9,-94,4
```

## Known limitations

- No blending between cascades, the resolution change can be visible on
  large flat surfaces.
- Casters further than 4 cascade radii from the view towards the light are
  lost (tall mountains with a low sun).
- The split distances follow the camera far plane, so a far plane much larger
  than the scene wastes resolution.
