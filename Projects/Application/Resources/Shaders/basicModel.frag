#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec3 outNormal;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 0) uniform texture2D textureAlbedo;
layout(set = 1, binding = 1) uniform texture2D textureSpecular;
layout(set = 1, binding = 2) uniform texture2D textureNormal;
layout(set = 1, binding = 3) uniform sampler basicSampler;

layout(std140, set = 0, binding = 0) uniform block {CameraData camera;};


    vec3 lightColor = vec3(1.0);
    vec3 lightPos = vec3(0.0, 1.0, 0.0);

    float lConstant = 1.0;
    float lLinear = 0.14;
    float lQuadratic = 0.07;

void main() {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 lightVec = lightPos - fragPos;
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(lightVec);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(vec3(camera.position) - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    float lightVecLength = length(lightVec);
    float attenuation = 1.0/(lConstant + lLinear * lightVecLength + lQuadratic * (lightVecLength * lightVecLength));
    vec3 result = (ambient * attenuation + diffuse * attenuation + specular * attenuation) * vec3(texture(sampler2D(textureAlbedo, basicSampler), outTexCoord));
    outColor = vec4(result, 1.0);
}