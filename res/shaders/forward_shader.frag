#version 330 core
#define NB_POINT_LIGHTS 4
#define MAX_CASCADES 8

out vec4 FragColor;

in VS_OUT {
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} fs_in;

struct Material {
    sampler2D albedoMap;
    sampler2D specularMap;
    sampler2D normalMap;
    vec3 albedo;
    vec3 specular;
    float shininess;
    bool hasNormalMap;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Surface values shared by every light
struct Surface {
    vec3 position;
    vec3 normal;
    vec3 albedo;
    vec3 specular;
};

uniform Material uMaterial;
uniform DirLight uDirLight;
uniform PointLight uPointLights[NB_POINT_LIGHTS];
uniform SpotLight uSpotLight;
uniform vec3 uViewPos;
uniform mat4 uView;

uniform sampler2DArray uShadowMap;
uniform mat4 uLightSpaceMatrices[MAX_CASCADES];
uniform float uCascadeFarPlanes[MAX_CASCADES];
uniform float uCascadeTexelSizes[MAX_CASCADES];
uniform float uCascadeTexelDepths[MAX_CASCADES];
uniform int uCascadeCount;
uniform bool uShowCascades;

// Depth bias in texels: a constant part for precision, plus a part growing
// with the slope of the surface seen from the light, where one texel covers
// a larger depth range.
const float SHADOW_BIAS_CONSTANT = 1.0;
const float SHADOW_BIAS_SLOPE = 2.0;
const float SHADOW_BIAS_MAX_SLOPE = 10.0;

vec3 blinnPhong(Surface s, vec3 lightDir, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
    float diff = max(dot(s.normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = diff > 0.0 ? pow(max(dot(s.normal, halfway), 0.0), uMaterial.shininess) : 0.0;
    return s.albedo * diffuseColor * diff + s.specular * specularColor * spec;
}

vec3 calcDirLight(DirLight light, Surface s, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 ambient = s.albedo * light.ambient;
    return ambient + (1.0 - shadow) * blinnPhong(s, lightDir, viewDir, light.diffuse, light.specular);
}

vec3 calcPointLight(PointLight light, Surface s, vec3 viewDir)
{
    vec3 toLight = light.position - s.position;
    float distance = length(toLight);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    vec3 ambient = s.albedo * light.ambient;
    return (ambient + blinnPhong(s, toLight / distance, viewDir, light.diffuse, light.specular)) * attenuation;
}

vec3 calcSpotLight(SpotLight light, Surface s, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - s.position);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 ambient = s.albedo * light.ambient;
    return ambient + intensity * blinnPhong(s, lightDir, viewDir, light.diffuse, light.specular);
}

int selectCascade(vec3 worldPos)
{
    float depth = abs((uView * vec4(worldPos, 1.0)).z);
    for (int i = 0; i < uCascadeCount - 1; ++i)
    {
        if (depth < uCascadeFarPlanes[i])
            return i;
    }
    return uCascadeCount - 1;
}

// Returns 1.0 when fully in shadow, 0.0 when fully lit.
float calcShadow(vec3 worldPos, vec3 normal, int layer)
{
    if (uCascadeCount == 0)
        return 0.0;

    // Normal offset: sample from a point pushed off the surface by about one
    // texel of this cascade, more at grazing angles where acne is worst.
    vec3 lightDir = normalize(-uDirLight.direction);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float offset = uCascadeTexelSizes[layer] * (0.5 + 1.5 * (1.0 - cosTheta));
    vec4 lightSpacePos = uLightSpaceMatrices[layer] * vec4(worldPos + normal * offset, 1.0);

    vec3 coords = lightSpacePos.xyz / lightSpacePos.w * 0.5 + 0.5;
    if (coords.z > 1.0)
        return 0.0;

    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float slope = min(sinTheta / max(cosTheta, 1e-4), SHADOW_BIAS_MAX_SLOPE);
    float bias = uCascadeTexelDepths[layer] * (SHADOW_BIAS_CONSTANT + SHADOW_BIAS_SLOPE * slope);

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
    vec4 albedoSample = texture(uMaterial.albedoMap, fs_in.texCoords);
    if (albedoSample.a < 0.5)
        discard;

    // Two sided lighting: back faces use the flipped normal
    float side = gl_FrontFacing ? 1.0 : -1.0;
    vec3 geometryNormal = normalize(fs_in.normal) * side;

    Surface s;
    s.position = fs_in.fragPos;
    s.albedo = albedoSample.rgb * uMaterial.albedo;
    s.specular = texture(uMaterial.specularMap, fs_in.texCoords).rgb * uMaterial.specular;
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
            vec3 mapped = texture(uMaterial.normalMap, fs_in.texCoords).rgb * 2.0 - 1.0;
            s.normal = normalize(mat3(T, B, geometryNormal) * mapped);
        }
    }

    vec3 viewDir = normalize(uViewPos - s.position);
    int layer = selectCascade(s.position);
    float shadow = calcShadow(s.position, geometryNormal, layer);

    vec3 result = calcDirLight(uDirLight, s, viewDir, shadow);
    for (int i = 0; i < NB_POINT_LIGHTS; ++i)
        result += calcPointLight(uPointLights[i], s, viewDir);
    result += calcSpotLight(uSpotLight, s, viewDir);

    if (uShowCascades && uCascadeCount > 0)
        result *= cascadeTint(layer);

    FragColor = vec4(result, 1.0);
}
