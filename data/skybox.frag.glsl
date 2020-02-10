#version 450

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
  // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  FragColor = texture(skybox, TexCoords);
}
