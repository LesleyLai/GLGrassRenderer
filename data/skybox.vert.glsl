#version 450

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout(binding = 0) uniform CameraBufferObject {
  mat4 view;
  mat4 proj;
} camera;

void main()
{
  TexCoords = aPos;
  gl_Position = camera.proj * vec4(aPos, 1.0);
}  
