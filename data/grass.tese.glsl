#version 450

layout(quads, equal_spacing, ccw) in;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

patch in TESC_OUT
{
  vec4 v1;
  vec4 v2;
  vec4 up;
  vec4 dir;
} tese_in;

out TESE_OUT
{
  vec3 normal;
  vec2 uv;
} tese_out;

void main() {
  const float u = gl_TessCoord.x;
  const float v = gl_TessCoord.y;

  const vec3 v0 = gl_in[0].gl_Position.xyz;
  const vec3 v1 = tese_in.v1.xyz;
  const vec3 v2 = tese_in.v2.xyz;
  const vec3 dir = tese_in.dir.xyz;
  const float width = tese_in.v2.w;
  
  const vec3 a = v0 + v * (v1 - v0);
  const vec3 b = v1 + v * (v2 - v1);
  const vec3 c = a + v * (b - a);
  const vec3 t1 = dir;
  
  const vec3 c0 = c - t1 * width * 0.5;
  const vec3 c1 = c + t1 * width * 0.5;
  const vec3 t0 = normalize(b - a);

  const float t = u + 0.5 * v - u * v;
  const vec3 p = (1.0 - t) * c0 + t * c1;
  
  tese_out.uv = vec2(u, v);
  tese_out.normal = normalize(cross(t0, t1));
  gl_Position = camera.proj * camera.view * vec4(p, 1.0);
}
