#version 450

uniform mat4 view;
uniform mat4 proj;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
