#version 450

in VS_OUT
{
  vec4 v1;
  vec4 v2;
  vec4 up;
  vec4 dir;
} tesc_in[];

patch out TESC_OUT
{
  vec4 v1;
  vec4 v2;
  vec4 up;
  vec4 dir;
} tesc_out;

layout(vertices = 1) out;

void main() {
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  tesc_out.v1 = tesc_in[gl_InvocationID].v1;
  tesc_out.v2 = tesc_in[gl_InvocationID].v2;
  tesc_out.up = tesc_in[gl_InvocationID].up;
  tesc_out.dir = tesc_in[gl_InvocationID].dir;

  gl_TessLevelInner[0] = 2.0;
  gl_TessLevelInner[1] = 7.0;
  gl_TessLevelOuter[0] = 7.0;
  gl_TessLevelOuter[1] = 2.0;
  gl_TessLevelOuter[2] = 7.0;
  gl_TessLevelOuter[3] = 2.0;
}
