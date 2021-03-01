#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObjectCamera {
    mat4 view;
    mat4 proj;
} uboCamera;

layout(binding = 1) uniform UniformBufferObjectModel {
    mat4 model;
} uboModel;

layout(location = 0) out vec2 outTexCoord;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 texCoord;

void main() {
    gl_Position = uboCamera.proj * uboCamera.view * uboModel.model * vec4(inPosition, 1.0);
    outTexCoord = texCoord;
}
