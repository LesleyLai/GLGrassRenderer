#version 450

uniform mat4 model;

layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 2) in vec4 v2;
layout(location = 3) in vec4 up;


layout(location = 0) out vec4 tesc_v1;
layout(location = 1) out vec4 tesc_v2;
layout(location = 2) out vec4 tesc_up;
layout(location = 3) out vec4 tesc_dir;

void main() {
  vec4 pv0 = model * vec4(v0.xyz, 1.0);
  vec4 pv1 = model * vec4(v1.xyz, 1.0);
  vec4 pv2 = model * vec4(v2.xyz, 1.0);

  tesc_v1 = vec4(pv1.xyz, v1.w);
  tesc_v2 = vec4(pv2.xyz, v2.w);
  tesc_up = vec4(normalize(up.xyz), up.w);

  float angle = v0.w;

  vec3 dir = normalize(cross(tesc_up.xyz,
                             vec3(sin(angle),
                                  0,
                                  cos(angle))));
  
  tesc_dir = vec4(dir, 0.0);
  
  gl_Position = pv0;
}
