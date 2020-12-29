#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec3 outNormal;

layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 0) uniform texture2D texSampler;
layout(set = 1, binding = 1) uniform sampler basicSampler;



void main() {
    outColor = texture(sampler2D(texSampler, basicSampler), outTexCoord);
    //outColor = vec4(sampler2D(texSampler,sampler),outTexCoord, 1.0f, 1.0f);
}