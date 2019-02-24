#version 450

uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec4 tv1[];
layout(location = 1) in vec4 tv2[];
layout(location = 2) in vec4 tup[];
layout(location = 3) in vec4 twd[];

layout(location = 0) patch out vec4 tese_v1;
layout(location = 1) patch out vec4 tese_v2;
layout(location = 2) patch out vec4 tese_up;
layout(location = 3) patch out vec4 tese_dir;

layout(vertices = 1) out;

void main() {
  // Don't move the origin location of the patch
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  tese_v1 = tv1[0];
  tese_v2 = tv2[0];
  tese_up = tup[0];
  tese_dir = twd[0];

  gl_TessLevelInner[0] = 1.0;
  gl_TessLevelInner[1] = 7.0;
  gl_TessLevelOuter[0] = 7.0;
  gl_TessLevelOuter[1] = 1.0;
  gl_TessLevelOuter[2] = 7.0;
  gl_TessLevelOuter[3] = 1.0;
}
