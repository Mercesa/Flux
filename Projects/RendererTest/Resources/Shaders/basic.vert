#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 outTexCoord;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

void main() {
    gl_Position = vec4(inPosition.xyz, 1.0);
    outTexCoord = texCoord;
}