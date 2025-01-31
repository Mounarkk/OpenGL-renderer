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

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};

uniform Material material;
//uniform PointLight light;
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main() {
   // attenuation
   // float distance = length(light.position - FragPos);
   // float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

   // ambient lighting
   vec3 ambient = spotLight.ambient * vec3(texture(material.diffuse, TexCoords));


   vec3 norm = normalize(Normal);
   vec3 lightDir = normalize(spotLight.position - FragPos);


   float theta = dot(lightDir, normalize(-spotLight.direction)); // Since we use camera.Front, the vector is pointing towards the viewer

   if (theta > spotLight.cutOff) {
      // diffuse lighting
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = spotLight.diffuse * (vec3(texture(material.diffuse, TexCoords)) * diff);

      // specular lighting
      vec3 viewDir = normalize(viewPos - FragPos);
      vec3 reflectedDir = normalize(reflect(-lightDir, norm));
      float spec = pow(max(dot(viewDir, reflectedDir), 0.0), material.shininess);
      vec3 specular = (vec3(texture(material.specular, TexCoords)) * spec) * spotLight.specular;

      vec3 result = (ambient + diffuse + specular);

      FragColor = vec4(result, 1);
   } else {
      FragColor = vec4(ambient, 1.0);
   }

   // emission map
   //vec3 emission = vec3(texture(material.emission, TexCoords));
}