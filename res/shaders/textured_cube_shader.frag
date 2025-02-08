#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
   sampler2D texture_diffuse1;
   sampler2D texture_specular1;
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

#define NB_POINT_LIGHTS 4

uniform Material material;
uniform DirLight dirLight;
uniform PointLight[NB_POINT_LIGHTS] pointLights;
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
   // properties
   vec3 norm = normalize(Normal);
   vec3 viewDir = normalize(viewPos - FragPos);

   // phase 1 : Directional lighting
   vec3 result = CalcDirLight(dirLight, norm, viewDir);

   // phase 2: Point lights
   for (int i = 0; i < NB_POINT_LIGHTS; i++) {
      result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
   }

   // phase 3: Spot light
   result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

   FragColor = vec4(result, 1.0);

   // emission map
   //vec3 emission = vec3(texture(material.emission, TexCoords));
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
   vec3 lightDir = normalize(light.direction);

   // ambient lighting
   vec3 ambient = vec3(texture(material.texture_diffuse1, TexCoords)) * light.ambient;

   // diffuse lighting
   float diff = max(dot(-lightDir, normal), 0.0);
   vec3 diffuse = vec3(texture(material.texture_diffuse1, TexCoords)) * light.diffuse * diff;

   // specular lighting
   vec3 reflectedDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
   vec3 specular = vec3(texture(material.texture_specular1, TexCoords)) * spec * light.specular;

   return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
   vec3 lightDir = normalize(FragPos - light.position);

   // attenuation
   float distance = length(light.position - FragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

   // ambient lighting
   vec3 ambient = vec3(texture(material.texture_diffuse1, TexCoords)) * light.ambient;

   // diffuse
   float diff = max(dot(-lightDir, normal), 0.0);
   vec3 diffuse = vec3(texture(material.texture_diffuse1, TexCoords)) * light.diffuse * diff;

   // specular lighting
   vec3 reflectedDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
   vec3 specular = vec3(texture(material.texture_specular1, TexCoords)) * spec * light.specular;

   return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
   vec3 lightDir = normalize(light.position - FragPos);


   float theta = dot(lightDir, normalize(-light.direction)); // Since we use camera.Front, the vector is pointing towards the viewer
   float epsilon = spotLight.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

   // ambient lighting
   vec3 ambient = vec3(texture(material.texture_diffuse1, TexCoords)) * light.ambient;

   // diffuse lighting
   float diff = max(dot(normal, lightDir), 0.0);
   vec3 diffuse = vec3(texture(material.texture_diffuse1, TexCoords)) * light.diffuse * diff * intensity;

   // specular lighting
   vec3 reflectedDir = normalize(reflect(-lightDir, normal));
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
   vec3 specular = vec3(texture(material.texture_specular1, TexCoords)) * spec * light.specular * intensity;

   return ambient + diffuse + specular;
}