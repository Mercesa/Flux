#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(std140, set = 0, binding = 0) uniform lightMatrix {mat4 directionalLightMatrix;};

layout(push_constant) uniform PushConsts {
    mat4 model;
} pushConsts;

void main() {
	gl_Position = directionalLightMatrix * pushConsts.model * vec4(inPosition, 1.0);
}