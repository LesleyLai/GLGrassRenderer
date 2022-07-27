#version 450

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
  FragColor = texture(skybox, TexCoords);
}
