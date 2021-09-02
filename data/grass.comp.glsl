#version 450

#define WORKGROUP_SIZE 32
layout(local_size_x = WORKGROUP_SIZE,
local_size_y = 1,
local_size_z = 1) in;

layout(binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

uniform float current_time;
uniform float delta_time;

uniform float wind_magnitude;
uniform float wind_wave_length;
uniform float wind_wave_period;

struct Blade {
    vec4 v0;
    vec4 v1;
    vec4 v2;
    vec4 up;
};

layout(binding = 1, std140) buffer inputBuffer {
    Blade inputBlades[];
};

layout(binding = 2, std140) buffer outputBuffer {
    Blade outputBlades[];
};

// Indirect drawing count
layout(binding = 3) buffer NumBlades {
    uint vertexCount;
    uint instanceCount;// = 1
    uint firstVertex;// = 0
    uint firstInstance;// = 0
} numBlades;

bool inBounds(float value, float bounds) {
    return (value >= -bounds) && (value <= bounds);
}

float rand(float seed) {
    return fract(sin(seed)*100000.0);
}

void main() {
    // Reset the number of blades to 0
    if (gl_GlobalInvocationID.x == 0) {
        numBlades.vertexCount = 0;
    }
    barrier();// Wait till all threads reach this point

    uint index = gl_GlobalInvocationID.x;
    vec3 v0 = inputBlades[index].v0.xyz;
    vec3 v1 = inputBlades[index].v1.xyz;
    vec3 v2 = inputBlades[index].v2.xyz;
    vec3 up = normalize(inputBlades[index].up.xyz);
    float orientation = inputBlades[index].v0.w;
    float height = inputBlades[index].v1.w;
    float width = inputBlades[index].v2.w;
    float stiffness = inputBlades[index].up.w;

    // Frustum culling
    vec4 v0ClipSpace = camera.proj * camera.view * vec4(v0, 1);
    vec4 v1ClipSpace = camera.proj * camera.view * vec4(v1, 1);
    v0ClipSpace /= v0ClipSpace.w;
    v1ClipSpace /= v1ClipSpace.w;

    bool v0OutFrustum =
    v0ClipSpace.x < -1 || v0ClipSpace.x > 1
    || v0ClipSpace.y < -1 || v0ClipSpace.y > 1;

    bool v1OutFrustum =
    v1ClipSpace.x < -1 || v1ClipSpace.x > 1
    || v1ClipSpace.y < -1 || v1ClipSpace.y > 1;
    if (v0OutFrustum && v1OutFrustum) return;

    // Distance culling
    const float far1 = 0.95;
    if (v0ClipSpace.z > far1 && v1ClipSpace.z > far1 && rand(index) > 0.5) {
        return;
    }
    const float far2 = 0.98;
    if (v0ClipSpace.z > far2 && v1ClipSpace.z > far2 && rand(index) > 0.2) {
        return;
    }
    const float far3 = 0.99;
    if (v0ClipSpace.z > far3 && v1ClipSpace.z > far3 && rand(index) > 0.1) {
        return;
    }

    // Apply forces {
    //  Gravities
    vec3 gE = vec3(0, -0.98, 0);
    vec3 widthDir = vec3(cos(orientation), 0, sin(orientation));
    vec3 bladeFace = normalize(cross(up, widthDir));
    vec3 gF = 0.25*length(gE)*bladeFace;
    vec3 g = gE + gF;

    //  Recovery
    vec3 r = (v0 + up * height - v2) * stiffness;

    //  Wind
    vec3 windForce = 0.25 * wind_magnitude *
    vec3(
    sin(current_time * 3. / wind_wave_period + v0.x * 0.1 * 11 / wind_wave_length),
    0,
    sin(current_time * 3. / wind_wave_period + v0.z * 0.2 * 11 / wind_wave_length) * 0.1
    );
    float fd = 1 - abs(dot(normalize(windForce), normalize(v2 - v0)));
    float fr = dot((v2 - v0), up) / height;
    vec3 w = windForce * fd * fr;

    v2 += (0.1 * g + r + w) * delta_time;

    float lproj = length(v2 - v0 - up * dot((v2-v0), up));
    v1 = v0 + height*up*max(1-lproj/height, 0.05*max(lproj/height, 1));

    inputBlades[index].v1.xyz = v1;
    inputBlades[index].v2.xyz = v2;
    // }

    outputBlades[atomicAdd(numBlades.vertexCount, 1)] = inputBlades[index];
}
