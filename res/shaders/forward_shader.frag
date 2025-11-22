#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosDirLightSpace;
in vec2 TexCoords;
in mat3 TBN;

struct Material {
   sampler2D albedoMap;
   sampler2D specularMap;
   sampler2D normalMap;
   //sampler2D emission;
   float shininess;
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

float near = 0.1;
float far = 100;

#define NB_POINT_LIGHTS 4

uniform Material uMaterial;
uniform DirLight uDirLight;
uniform PointLight[NB_POINT_LIGHTS] uPointLights;
uniform SpotLight uSpotLight;
uniform vec3 objectColor;
uniform vec3 viewPos;

uniform sampler2D uDirLightShadowMap;

float LinearizeDepthValue(float z);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace);

void main() {
   // properties
   //vec3 norm = normalize(Normal);
   vec3 norm = vec3(texture(uMaterial.normalMap, TexCoords)); // Normal from tangent space
   norm = norm * 2 - 1.0;
   norm = normalize(TBN * norm); // Transform the normal to world space
   vec3 viewDir = normalize(viewPos - FragPos);

   // Is the fragment in the dirlight shadow ?
   float dirLightShadow = ShadowCalculation(FragPosDirLightSpace);

   // phase 1 : Directional lighting
   vec3 result = CalcDirLight(uDirLight, norm, viewDir, dirLightShadow);

   // phase 2: Point lights
   for (int i = 0; i < NB_POINT_LIGHTS; i++) {
      result += CalcPointLight(uPointLights[i], norm, FragPos, viewDir);
   }

   // phase 3: Spot light
   result += CalcSpotLight(uSpotLight, norm, FragPos, viewDir);

   FragColor = vec4(result, 1.0);

   // emission map
   //vec3 emission = vec3(texture(uMaterial.emission, TexCoords));
}

// To see what the depth buffer looks like
float LinearizeDepthValue(float z) {
   float ndc = z * 2.0 - 1.0;
   float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
   linearDepth = linearDepth / far;

   return linearDepth;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
   vec3 lightDir = normalize(light.direction);

   // ambient lighting
   vec3 ambient = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.ambient;

   // diffuse lighting
   float diff = max(dot(-lightDir, normal), 0.0);
   vec3 diffuse = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.diffuse * diff;

   // specular lighting
   vec3 reflectedDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), uMaterial.shininess);
   vec3 specular = vec3(texture(uMaterial.specularMap, TexCoords)) * spec * light.specular;

   return ambient + (1 - shadow) * (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
   vec3 lightDir = normalize(FragPos - light.position);

   // attenuation
   float distance = length(light.position - FragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

   // ambient lighting
   vec3 ambient = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.ambient;

   // diffuse
   float diff = max(dot(-lightDir, normal), 0.0);
   vec3 diffuse = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.diffuse * diff;

   // specular lighting
   vec3 reflectedDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), uMaterial.shininess);
   vec3 specular = vec3(texture(uMaterial.specularMap, TexCoords)) * spec * light.specular;

   return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
   vec3 lightDir = normalize(light.position - FragPos);


   float theta = dot(lightDir, normalize(-light.direction)); // Since we use camera.Front, the vector is pointing towards the viewer
   float epsilon = uSpotLight.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

   // ambient lighting
   vec3 ambient = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.ambient;

   // diffuse lighting
   float diff = max(dot(normal, lightDir), 0.0);
   vec3 diffuse = vec3(texture(uMaterial.albedoMap, TexCoords)) * light.diffuse * diff * intensity;

   // specular lighting
   vec3 reflectedDir = normalize(reflect(-lightDir, normal));
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), uMaterial.shininess);
   vec3 specular = vec3(texture(uMaterial.specularMap, TexCoords)) * spec * light.specular * intensity;

   return ambient + diffuse + specular;
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
   projCoords = projCoords * 0.5 + 0.5;

   // If outside shadow map bounds, assume no shadow
   if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
   projCoords.y < 0.0 || projCoords.y > 1.0)
   return 0.0;

   float closestDepth = texture(uDirLightShadowMap, projCoords.xy).r;
   float currentDepth = projCoords.z;
   float bias = 0.007;
   float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

   return shadow;
}
