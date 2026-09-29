# Physically based shading

How surfaces are lit: the metallic-roughness material model, the BRDF, the
conversion of older formats, and tone mapping.

Code: `src/rendering/Material.*`, `res/shaders/common/brdf.glsl`,
`forward_shader.frag`, `ResourceManager::loadMaterial` and `screen.frag`.

## 1. Why change from Blinn-Phong

Blinn-Phong has a diffuse color, a specular color and a shininess exponent,
three knobs with no physical meaning. Nothing stops a surface from reflecting
more light than it receives, and a material tuned under one light looks wrong
under another.

Physically based shading describes what the surface *is* rather than how it
should look, and derives the look from a model of light reflection that
conserves energy. The same material then behaves correctly under the sun, a
torch or a dim ambient. It is also the model used by every modern asset
format and tool (glTF, Substance, Blender, game engines), so assets come in
with the right parameters.

## 2. The material: metallic-roughness

The glTF 2.0 model, stored in `Material`:

| Parameter | Meaning | Map |
|---|---|---|
| albedo | Base color: diffuse color of dielectrics, reflection tint of metals | sRGB RGB, alpha for cut-outs |
| metallic | 0 for dielectrics (wood, stone, cloth, paint), 1 for bare metal | B channel of the packed map |
| roughness | 0 is a mirror, 1 is fully blurry reflections | G channel of the packed map |
| normal | Surface detail | tangent space normal map |
| occlusion | Crevices that ambient light does not reach | R channel |
| emissive | Light the surface emits on its own | sRGB RGB |

Each map is multiplied by a factor, and a missing map is a neutral 1x1
texture, so the factors alone are a valid material.

**Dielectrics and metals** reflect light in very different ways:

- A dielectric reflects about 4% of the light at normal incidence, with no
  tint, and scatters the rest under its surface, which gives the diffuse
  color.
- A metal absorbs whatever is not reflected: it has no diffuse at all, and its
  reflection is tinted by the albedo (gold reflects yellow).

The shader turns the parameters into the two quantities the lighting needs:

```glsl
s.diffuseColor = albedo * (1.0 - metallic);            // metals: none
s.f0 = mix(vec3(0.08 * specular), albedo, metallic);   // reflectance
```

`f0` is the reflectance at normal incidence. `specular` defaults to 0.5,
giving the 4% of common dielectrics.

## 3. The BRDF

The BRDF (bidirectional reflectance distribution function) tells how much of
the light arriving from one direction leaves towards the viewer. The model
used is Cook-Torrance, which treats the surface as a field of tiny mirrors,
the microfacets:

```
specular = D * V * F
diffuse  = (1 - F) * diffuseColor / PI
outgoing = (diffuse + specular) * radiance * NdotL
```

- **D, the distribution** (GGX): the proportion of microfacets oriented to
  reflect the light towards the viewer, i.e. facing the half vector between
  the two. Rough surfaces spread them out: wide, dim highlights. Smooth
  surfaces align them: small, sharp highlights.
- **V, the visibility** (height-correlated Smith): microfacets shadow and hide
  each other at grazing angles. It also includes the normalization of the
  microfacet model.
- **F, Fresnel** (Schlick): reflection grows towards 100% at grazing angles,
  for every material. It is why a lake reflects the sky far away but not at
  your feet.

The diffuse term is Lambert, divided by PI so that a white surface never
sends back more light than it received. It only receives what the specular
lobe did not reflect (`1 - F`).

Roughness is squared before use (`alpha = roughness^2`): artists' roughness
is perceptually linear, the lobes are not.

**Units.** Because of the division by PI, a light of intensity PI on a white
surface facing it gives an outgoing value of 1. The default intensities are
about PI times those of the Blinn-Phong version.

## 4. Ambient light

Real ambient light comes from the whole environment and is the job of image
based lighting, not implemented yet. Until then the ambient term is a
hemisphere light: the sun's ambient color from above, fading to 30% of it
from below. It keeps surfaces out of the sun readable, since their brightness
still depends on their orientation.

It is weakest on metals: they reflect the environment, and there is no
environment yet. A smooth metal with no light in its reflection direction is
nearly black, which is physically right for this lighting and the main reason
image based lighting is the next step.

## 5. Converting older formats

OBJ and other Phong-based formats have no metallic or roughness. The loader
converts:

| Phong | Metallic-roughness |
|---|---|
| `Kd` (diffuse color) | albedo, when there is no diffuse map |
| `map_Ks` or `Ks` | `specular` (dielectric reflectance) |
| `Ns` (exponent) | roughness = sqrt(2 / (Ns + 2)), clamped to [0.2, 1] |
| `Ke`, `map_Ke` | emissive |
| metallic | always 0 |

The roughness formula gives a GGX lobe about as wide as the Blinn-Phong one
of that exponent. The clamp avoids the mirror-like surfaces that very high
exponents written by exporters would produce.

glTF files are detected by the metallic factor Assimp sets for them, and use
their parameters directly, including the alpha mode (`OPAQUE` ignores alpha,
`MASK` uses the cutoff, `BLEND` is approximated with a cutoff).

## 6. Tone mapping

Lighting is computed in linear HDR values into a 16-bit float target. A sunny
surface can be well above 1, a shadowed one far below. The screen needs
[0, 1], and simply clamping turns every bright area into flat white.

The post-process pass:

1. Multiplies by the exposure, `2^stops`, like a camera.
2. Applies a fit of the ACES filmic curve: nearly linear in the dark and
   midtones, then a soft shoulder that compresses highlights instead of
   clipping them.
3. Encodes to sRGB with a 2.2 gamma.

## 7. Checking materials

The debug panel, or `--debug-view <name>`, replaces the lit image with one
channel: albedo, normals, metallic, roughness, occlusion or emissive. These
views skip exposure and tone mapping to show the raw values.
