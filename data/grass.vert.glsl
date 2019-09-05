#version 450

layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 2) in vec4 v2;
layout(location = 3) in vec4 up;

out VS_OUT
{
  vec4 v1;
  vec4 v2;
  vec4 up;
  vec4 dir;
} vs_out;

void main() {
  vs_out.v1 = v1;
  vs_out.v2 = v2;
  vs_out.up = vec4(normalize(up.xyz), up.w);

  float angle = v0.w;

  vec3 dir = normalize(cross(vs_out.up.xyz,
                             vec3(sin(angle),
                                  0,
                                  cos(angle))));

  vs_out.dir = vec4(dir, 0.0);

  gl_Position = v0;
}
