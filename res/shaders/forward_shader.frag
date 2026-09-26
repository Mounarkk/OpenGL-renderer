#version 450 core
#include "common/frame.glsl"
#include "common/lights.glsl"
#include "common/shadows.glsl"

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
layout (binding = SPECULAR_UNIT) uniform sampler2D uSpecularMap;
layout (binding = NORMAL_UNIT) uniform sampler2D uNormalMap;

// Per material factors, set by Material::bind
struct Material {
    vec3 albedo;
    vec3 specular;
    float shininess;
    bool hasNormalMap;
};
uniform Material uMaterial;

// Surface values shared by every light
struct Surface {
    vec3 position;
    vec3 normal;
    vec3 albedo;
    vec3 specular;
};

// Slope part of the depth bias stops growing past this tangent
const float SHADOW_BIAS_MAX_SLOPE = 10.0;

vec3 blinnPhong(Surface s, vec3 lightDir, vec3 viewDir, vec3 radiance)
{
    float diff = max(dot(s.normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = diff > 0.0 ? pow(max(dot(s.normal, halfway), 0.0), uMaterial.shininess) : 0.0;
    return (s.albedo * diff + s.specular * spec) * radiance;
}

vec3 pointLightContribution(PointLight light, Surface s, vec3 viewDir)
{
    vec3 toLight = light.positionRange.xyz - s.position;
    float distance = length(toLight);
    float attenuation = rangeAttenuation(distance, light.positionRange.w);
    if (attenuation <= 0.0)
        return vec3(0.0);
    return blinnPhong(s, toLight / distance, viewDir, light.radiance.rgb) * attenuation;
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
    return blinnPhong(s, lightDir, viewDir, light.radianceInnerCos.rgb) * attenuation;
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
    vec4 albedoSample = texture(uAlbedoMap, fs_in.texCoords);
    if (albedoSample.a < 0.5)
        discard;

    // Two sided lighting: back faces use the flipped normal
    float side = gl_FrontFacing ? 1.0 : -1.0;
    vec3 geometryNormal = normalize(fs_in.normal) * side;

    Surface s;
    s.position = fs_in.fragPos;
    s.albedo = albedoSample.rgb * uMaterial.albedo;
    s.specular = texture(uSpecularMap, fs_in.texCoords).rgb * uMaterial.specular;
    s.normal = geometryNormal;

    if (uMaterial.hasNormalMap)
    {
        // Re-orthogonalize the interpolated basis (Gram-Schmidt). Meshes
        // without UVs have null tangents: keep the geometric normal then.
        vec3 T = fs_in.tangent - geometryNormal * dot(fs_in.tangent, geometryNormal);
        if (dot(T, T) > 1e-8)
        {
            T = normalize(T);
            vec3 B = normalize(cross(geometryNormal, T)) * sign(dot(cross(geometryNormal, T), fs_in.bitangent));
            vec3 mapped = texture(uNormalMap, fs_in.texCoords).rgb * 2.0 - 1.0;
            s.normal = normalize(mat3(T, B, geometryNormal) * mapped);
        }
    }

    vec3 viewDir = normalize(frame.cameraPosition.xyz - s.position);
    int layer = selectCascade(s.position);

    vec3 result = vec3(0.0);
    if (lights.counts.z != 0)
    {
        float shadow = sunShadow(s.position, geometryNormal, layer);
        vec3 sunDir = -normalize(lights.sun.direction.xyz);
        result += s.albedo * lights.sun.ambient.rgb;
        result += (1.0 - shadow) * blinnPhong(s, sunDir, viewDir, lights.sun.radiance.rgb);
    }
    for (int i = 0; i < lights.counts.x; ++i)
        result += pointLightContribution(lights.pointLights[i], s, viewDir);
    for (int i = 0; i < lights.counts.y; ++i)
        result += spotLightContribution(lights.spotLights[i], s, viewDir);

    if (shadows.flags.z != 0 && shadows.flags.x != 0)
        result *= cascadeTint(layer);

    FragColor = vec4(result, 1.0);
}
