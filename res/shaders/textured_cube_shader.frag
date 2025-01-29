#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
   sampler2D diffuse;
   sampler2D specular;
   sampler2D emission;
   float shininess;
};

struct Light {
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main() {
   // ambient lighting
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

   // diffuse lighting
   vec3 norm = normalize(Normal);
   vec3 lightDir = normalize(-light.direction);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse = light.diffuse * (vec3(texture(material.diffuse, TexCoords)) * diff);

   // specular lighting
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 reflectedDir = normalize(reflect(-lightDir, norm));
   float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
   vec3 specular = (vec3(texture(material.specular, TexCoords)) * spec) * light.specular;

   // emission map
   vec3 emission = vec3(texture(material.emission, TexCoords));

   vec3 result = ambient + diffuse + specular;

   FragColor = vec4(result, 1);
}