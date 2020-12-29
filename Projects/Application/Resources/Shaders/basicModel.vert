#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 0, binding = 1) uniform UniformBufferObjectModel {
    mat4 model;
} uboModel;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;


void main() {
    gl_Position = ubo.proj * ubo.view * uboModel.model * vec4(inPosition, 1.0);
    outTexCoord = texCoord;
    outNormal = normal;
}