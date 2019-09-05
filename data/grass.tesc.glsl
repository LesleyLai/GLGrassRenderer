#version 450

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

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
  
  float z1 = (camera.proj * camera.view * vec4(tesc_out.v1.xyz, 1)).z;
  float z2 = (camera.proj * camera.view * vec4(tesc_out.v2.xyz, 1)).z;


  if (z1 < 0.8 && z2 < 0.8) {
    gl_TessLevelInner[0] = 2.0;
    gl_TessLevelInner[1] = 7.0;
    gl_TessLevelOuter[0] = 7.0;
    gl_TessLevelOuter[1] = 2.0;
    gl_TessLevelOuter[2] = 7.0;
    gl_TessLevelOuter[3] = 2.0;
  } else {
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 3.0;
    gl_TessLevelOuter[0] = 3.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 3.0;
    gl_TessLevelOuter[3] = 1.0;
  }

}
