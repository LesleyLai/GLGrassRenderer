#version 450

layout(quads, equal_spacing, ccw) in;

uniform mat4 view;
uniform mat4 proj;

layout(location = 0) patch in vec4 tese_v1;
layout(location = 1) patch in vec4 tese_v2;
layout(location = 2) patch in vec4 tese_up;
layout(location = 3) patch in vec4 tese_dir;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;

void main() {
  const float u = gl_TessCoord.x;
  const float v = gl_TessCoord.y;

  const vec3 v0 = gl_in[0].gl_Position.xyz;
  const vec3 v1 = tese_v1.xyz;
  const vec3 v2 = tese_v2.xyz;
  const vec3 dir = tese_dir.xyz;
  const float width = tese_v2.w;
  
  const vec3 a = v0 + v * (v1 - v0);
  const vec3 b = v1 + v * (v2 - v1);
  const vec3 c = a + v * (b - a);
  const vec3 t1 = dir;
  
  const vec3 c0 = c - t1 * width * 0.5;
  const vec3 c1 = c + t1 * width * 0.5;
  const vec3 t0 = normalize(b - a);

  const float t = u + 0.5 * v - u * v;
  const vec3 p = (1.0 - t) * c0 + t * c1;
  
  uv = vec2(u, v);
  normal = normalize(cross(t0, t1));
  gl_Position = proj * view * vec4(p, 1.0);
}
