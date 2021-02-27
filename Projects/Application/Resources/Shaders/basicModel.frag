#version 460 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_KHR_vulkan_glsl : enable

#include "common.glsl"

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec3 outNormal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in mat3 TBN;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform texture2D textureAlbedo;
layout(set = 1, binding = 1) uniform texture2D textureSpecular;
layout(set = 1, binding = 2) uniform texture2D textureNormal;
layout(set = 1, binding = 3) uniform sampler basicSampler;


layout(std140, set = 0, binding = 0) uniform block {CameraData camera;};
layout (set = 0, binding = 1, std140) uniform Lights {
  Light lights[1024];
};

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance,
               in float b) // camera to point distance
{
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.6, 0.72, 0.909);
    return mix( rgb, fogColor, fogAmount );
}

void main() {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lights[0].color;



    vec3 norm = normalize(outNormal);
    norm = texture(sampler2D(textureNormal, basicSampler), outTexCoord).rgb;
    norm = norm * 2.0 - 1.0;
    norm = normalize(TBN * norm);
    vec3 result = vec3(0.0);

    vec3 viewDir = vec3(camera.position) - fragPos;
    float viewDistance = sqrt(dot(viewDir, viewDir));


    vec4 albedo = texture(sampler2D(textureAlbedo, basicSampler), outTexCoord);


    if(albedo.a < 0.99)
        discard;

    viewDir = normalize(viewDir);

    int amountOfLights = lights[0].amountOfLights;
    for(int a = 0; a < amountOfLights; ++a)
    {

        // diffuse
        vec3 lightVec = lights[a].position - fragPos;
        vec3 lightDir = normalize(lightVec);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lights[a].color;

        // specular
        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lights[a].color;

        float lightVecLength = length(lightVec);
        float attenuation = 1.0/(lights[a].constantFactor + lights[a].linearFactor * lightVecLength + lights[a].quadraticFactor * (lightVecLength * lightVecLength));
        result += (attenuation + diffuse * attenuation + specular * attenuation) * albedo.rgb;
    }

    vec3 color = applyFog(result, viewDistance, 0.0005);

    outColor = vec4(color.rgb, 1.0);
}