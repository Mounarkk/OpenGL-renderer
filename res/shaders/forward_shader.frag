#version 450 core
#include "common/frame.glsl"
#include "common/lights.glsl"
#include "common/shadows.glsl"
#include "common/brdf.glsl"

out vec4 FragColor;

in VS_OUT {
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} fs_in;

layout (binding = SHADOW_MAP_UNIT) uniform sampler2DArray uShadowMap;
layout (binding = ALBEDO_UNIT) uniform sampler2D uAlbedoMap;
layout (binding = NORMAL_UNIT) uniform sampler2D uNormalMap;
layout (binding = METALLIC_ROUGHNESS_UNIT) uniform sampler2D uMetallicRoughnessMap;
layout (binding = OCCLUSION_UNIT) uniform sampler2D uOcclusionMap;
layout (binding = EMISSIVE_UNIT) uniform sampler2D uEmissiveMap;
layout (binding = SPECULAR_UNIT) uniform sampler2D uSpecularMap;
layout (binding = IRRADIANCE_UNIT) uniform samplerCube uIrradianceMap;
layout (binding = PREFILTERED_UNIT) uniform samplerCube uPrefilteredMap;
layout (binding = BRDF_LUT_UNIT) uniform sampler2D uBrdfLut;

// Per material factors, multiplied with the maps. Set by Material::bind.
struct Material {
    vec4 albedo;
    float metallic;
    float roughness;
    float specular;     // dielectric reflectance, F0 = 0.08 * specular
    vec3 emissive;
    float alphaCutoff;  // negative for opaque materials
    bool hasNormalMap;
};
uniform Material uMaterial;

// DebugView of RendererSettings, 0 is the lit result
uniform int uDebugView;

// Slope part of the depth bias stops growing past this tangent
const float SHADOW_BIAS_MAX_SLOPE = 10.0;

vec3 pointLightContribution(PointLight light, Surface s, vec3 viewDir)
{
    vec3 toLight = light.positionRange.xyz - s.position;
    float distance = length(toLight);
    float attenuation = rangeAttenuation(distance, light.positionRange.w);
    if (attenuation <= 0.0)
        return vec3(0.0);
    return shadeSurface(s, toLight / distance, viewDir, light.radiance.rgb * attenuation);
}

vec3 spotLightContribution(SpotLight light, Surface s, vec3 viewDir)
{
    vec3 toLight = light.positionRange.xyz - s.position;
    float distance = length(toLight);
    vec3 lightDir = toLight / distance;

    float cosOuter = light.directionOuterCos.w;
    float cosInner = light.radianceInnerCos.w;
    float theta = dot(-lightDir, normalize(light.directionOuterCos.xyz));
    float cone = smoothstep(cosOuter, cosInner, theta);

    float attenuation = cone * rangeAttenuation(distance, light.positionRange.w);
    if (attenuation <= 0.0)
        return vec3(0.0);
    return shadeSurface(s, lightDir, viewDir, light.radianceInnerCos.rgb * attenuation);
}

// Image based lighting: light arriving from the whole environment.
// Diffuse comes from the irradiance map. Specular uses the split sum: the
// environment prefiltered for this roughness, in the reflection direction,
// times the integral of the BRDF, f0 * scale + bias, from the lookup table.
//
// The microfacet BRDF only counts light reflected once. On rough surfaces a
// good part bounces between microfacets before leaving and is lost: up to
// 40% for a rough metal. Fdez-Aguera 2019, "A Multiple-Scattering
// Microfacet Model for Real-Time Image-based Lighting", adds it back from
// the same lookup table, as a uniform lobe lit by the irradiance.
// Ambient occlusion tells how much of the hemisphere is hidden, which suits
// diffuse light. A reflection only looks in a cone around one direction,
// narrow on smooth surfaces, so it is less likely to be hidden when seen
// face-on. Lagarde and de Rousiers 2014, "Moving Frostbite to PBR".
float specularOcclusion(float NdotV, float occlusion, float roughness)
{
    float alpha = roughness * roughness;
    return clamp(pow(NdotV + occlusion, exp2(-16.0 * alpha - 1.0)) - 1.0 + occlusion, 0.0, 1.0);
}

vec3 environmentLight(Surface s, vec3 viewDir, vec2 scaleBias, vec3 geometryNormal)
{
    // Single scattering: the split sum
    vec3 FssEss = s.f0 * scaleBias.x + scaleBias.y;

    // Multiple scattering. A white surface reflects Ess in a single bounce,
    // the remaining Ems leaves after more bounces, each one keeping the
    // average Fresnel reflectance Favg: a geometric series
    float Ess = scaleBias.x + scaleBias.y;
    float Ems = 1.0 - Ess;
    vec3 Favg = s.f0 + (1.0 - s.f0) / 21.0;
    vec3 FmsEms = Ems * FssEss * Favg / (1.0 - Ems * Favg);

    vec3 reflected = reflect(-viewDir, s.normal);
    float level = s.roughness * lights.environment.z;
    vec3 radiance = textureLod(uPrefilteredMap, reflected, level).rgb;
    vec3 irradiance = texture(uIrradianceMap, s.normal).rgb;

    // Normal maps can bend the reflection under the actual surface, where
    // it would read the environment through the object
    float horizon = min(1.0 + dot(reflected, geometryNormal), 1.0);
    float NdotV = max(dot(s.normal, viewDir), 1e-4);
    float specularVisibility = specularOcclusion(NdotV, s.occlusion, s.roughness) * horizon * horizon;

    // What the specular reflects, in one bounce or more, does not diffuse
    vec3 diffuse = s.diffuseColor * (1.0 - FssEss - FmsEms);
    vec3 light = FssEss * radiance * specularVisibility +
                 (FmsEms + diffuse) * irradiance * s.occlusion;
    return light * lights.environment.x;
}

int selectCascade(vec3 worldPos)
{
    float depth = abs((frame.view * vec4(worldPos, 1.0)).z);
    for (int i = 0; i < CASCADE_COUNT - 1; ++i)
    {
        if (depth < shadows.cascades[i].x)
            return i;
    }
    return CASCADE_COUNT - 1;
}

// Returns 1.0 when fully in shadow, 0.0 when fully lit.
float sunShadow(vec3 worldPos, vec3 normal, int layer)
{
    if (shadows.flags.x == 0)
        return 0.0;

    // Normal offset: sample from a point pushed off the surface by about one
    // texel of this cascade, more at grazing angles where acne is worst.
    vec3 lightDir = -normalize(lights.sun.direction.xyz);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float offset = shadows.bias.z * shadows.cascades[layer].y * (0.5 + 1.5 * (1.0 - cosTheta));
    vec4 lightSpacePos = shadows.lightSpaceMatrices[layer] * vec4(worldPos + normal * offset, 1.0);

    vec3 coords = lightSpacePos.xyz / lightSpacePos.w * 0.5 + 0.5;
    if (coords.z > 1.0)
        return 0.0;

    // Depth bias in texels: a constant part for precision, plus a part
    // growing with the slope of the surface seen from the light, where one
    // texel covers a larger depth range.
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float slope = min(sinTheta / max(cosTheta, 1e-4), SHADOW_BIAS_MAX_SLOPE);
    float bias = shadows.cascades[layer].z * (shadows.bias.x + shadows.bias.y * slope);

    if (shadows.flags.y == 0)
    {
        float closest = texture(uShadowMap, vec3(coords.xy, layer)).r;
        return coords.z - bias > closest ? 1.0 : 0.0;
    }

    // 3x3 percentage closer filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0).xy);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closest = texture(uShadowMap, vec3(coords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += coords.z - bias > closest ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

vec3 cascadeTint(int layer)
{
    const vec3 colors[4] = vec3[](vec3(1.0, 0.4, 0.4), vec3(0.4, 1.0, 0.4),
                                  vec3(0.4, 0.4, 1.0), vec3(1.0, 1.0, 0.4));
    return colors[layer % 4];
}

void main()
{
    vec2 uv = fs_in.texCoords;
    vec4 albedo = texture(uAlbedoMap, uv) * uMaterial.albedo;
    if (albedo.a < uMaterial.alphaCutoff)
        discard;

    // glTF packing: roughness in G, metallic in B
    vec4 metallicRoughness = texture(uMetallicRoughnessMap, uv);
    float metallic = clamp(metallicRoughness.b * uMaterial.metallic, 0.0, 1.0);
    float roughness = clamp(metallicRoughness.g * uMaterial.roughness, 0.0, 1.0);
    float specular = texture(uSpecularMap, uv).r * uMaterial.specular;

    // Two sided lighting: back faces use the flipped normal
    float side = gl_FrontFacing ? 1.0 : -1.0;
    vec3 geometryNormal = normalize(fs_in.normal) * side;

    Surface s;
    s.position = fs_in.fragPos;
    s.normal = geometryNormal;
    s.roughness = roughness;
    s.occlusion = texture(uOcclusionMap, uv).r;
    // Metals have no diffuse and tint their reflection with the albedo
    s.diffuseColor = albedo.rgb * (1.0 - metallic);
    s.f0 = mix(vec3(0.08 * specular), albedo.rgb, metallic);

    if (uMaterial.hasNormalMap)
    {
        // Re-orthogonalize the interpolated basis (Gram-Schmidt). Meshes
        // without UVs have null tangents: keep the geometric normal then.
        vec3 T = fs_in.tangent - geometryNormal * dot(fs_in.tangent, geometryNormal);
        if (dot(T, T) > 1e-8)
        {
            T = normalize(T);
            vec3 B = normalize(cross(geometryNormal, T)) * sign(dot(cross(geometryNormal, T), fs_in.bitangent));
            vec3 mapped = texture(uNormalMap, uv).rgb * 2.0 - 1.0;
            s.normal = normalize(mat3(T, B, geometryNormal) * mapped);
        }
    }

    vec3 emissive = texture(uEmissiveMap, uv).rgb * uMaterial.emissive;

    if (uDebugView != 0)
    {
        // Colors are shown as they are, data is pre-darkened so that the
        // gamma correction of the post-process pass displays the raw value
        vec3 debug;
        if (uDebugView == 1) debug = albedo.rgb;
        else if (uDebugView == 2) debug = pow(s.normal * 0.5 + 0.5, vec3(2.2));
        else if (uDebugView == 3) debug = vec3(pow(metallic, 2.2));
        else if (uDebugView == 4) debug = vec3(pow(roughness, 2.2));
        else if (uDebugView == 5) debug = vec3(pow(s.occlusion, 2.2));
        else debug = emissive;
        FragColor = vec4(debug, 1.0);
        return;
    }

    vec3 viewDir = normalize(frame.cameraPosition.xyz - s.position);
    int layer = selectCascade(s.position);

    // Integral of the specular BRDF for this angle and roughness
    float NdotV = max(dot(s.normal, viewDir), 1e-4);
    vec2 scaleBias = texture(uBrdfLut, vec2(NdotV, s.roughness)).rg;

    // The same energy loss applies to direct lights. Scaling their lobe by
    // the missing fraction is the approximation Filament uses.
    float Ess = scaleBias.x + scaleBias.y;
    s.energyCompensation = 1.0 + s.f0 * (1.0 / max(Ess, 1e-3) - 1.0);

    vec3 result = environmentLight(s, viewDir, scaleBias, geometryNormal);
    if (lights.counts.z != 0)
    {
        float shadow = sunShadow(s.position, geometryNormal, layer);
        vec3 sunDir = -normalize(lights.sun.direction.xyz);
        result += (1.0 - shadow) * shadeSurface(s, sunDir, viewDir, lights.sun.radiance.rgb);
    }
    for (int i = 0; i < lights.counts.x; ++i)
        result += pointLightContribution(lights.pointLights[i], s, viewDir);
    for (int i = 0; i < lights.counts.y; ++i)
        result += spotLightContribution(lights.spotLights[i], s, viewDir);

    result += emissive;

    if (shadows.flags.z != 0 && shadows.flags.x != 0)
        result *= cascadeTint(layer);

    FragColor = vec4(result, 1.0);
}
