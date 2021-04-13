#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"

layout(std140, set = 0, binding = 0) uniform block {CameraData camera;};


layout(push_constant) uniform PushConsts {
    mat4 model;
} pushConsts;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec4 fragPosLightSpace;
layout(location = 4) out mat3 TBN;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(std140, set = 2, binding = 0) uniform lightMatrix {mat4 directionalLightMatrix;};


void main() {
    fragPos = vec3(pushConsts.model * vec4(inPosition, 1.0));
    gl_Position = camera.proj * camera.view * pushConsts.model * vec4(inPosition, 1.0);
    outTexCoord = texCoord;
    outNormal = normal;

   vec3 T = normalize(vec3(pushConsts.model * vec4(tangent,   0.0)));
   vec3 B = normalize(vec3(pushConsts.model * vec4(bitangent, 0.0)));
   vec3 N = normalize(vec3(pushConsts.model * vec4(normal,    0.0)));

   TBN = mat3(T, B, N);

   fragPosLightSpace = directionalLightMatrix * vec4(fragPos, 1.0);
}