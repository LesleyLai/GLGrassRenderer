#version 450

layout(binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec3 position;
} camera;

in TESE_OUT
{
  vec3 position;
  vec3 normal;
  vec2 uv;
} frag_in;

layout(location = 0) out vec4 outColor;

void main() {
  vec2 uv = frag_in.uv;
  vec3 normal = normalize(frag_in.normal);

  vec3 upperColor = vec3(0.4,1,0.1);
  vec3 lowerColor = vec3(0.0,0.2,0.1);

  vec3 sunLightDirection = normalize(vec3(1.0, 1.0, 1.0));

  vec3 viewPos = camera.position;
  vec3 FragPos = frag_in.position;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-sunLightDirection, normal);
  vec3 halfwayDir = normalize(sunLightDirection + viewDir);

  // 0: bottom, 1: top
  float bottomToTop = uv.y;
  vec3 mixedColor = mix(lowerColor, upperColor, bottomToTop);

  // ambient
  float ambientStrength = 0.4;
  vec3 ambient = ambientStrength * mixedColor;

  // diffuse
  float diff = clamp(dot(normal, sunLightDirection), 0.5, 1.0) * 0.4;
  vec3 diffuse = diff * mixedColor;

  // specular
  float specularStrength = 0.2;
  float shininess = 8.0;
  float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
  vec3 specular = specularStrength * vec3(spec);

  vec3 result = ambient + diffuse + specular;

  outColor = vec4(result, 1.0);
}
