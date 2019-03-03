#version 450

uniform mat4 model;

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
  vec4 pv0 = model * vec4(v0.xyz, 1.0);
  vec4 pv1 = model * vec4(v1.xyz, 1.0);
  vec4 pv2 = model * vec4(v2.xyz, 1.0);

  vs_out.v1 = vec4(pv1.xyz, v1.w);
  vs_out.v2 = vec4(pv2.xyz, v2.w);
  vs_out.up = vec4(normalize(up.xyz), up.w);

  float angle = v0.w;

  vec3 dir = normalize(cross(vs_out.up.xyz,
                             vec3(sin(angle),
                                  0,
                                  cos(angle))));
  
  vs_out.dir = vec4(dir, 0.0);
  
  gl_Position = pv0;
}
