#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

vec3 LessThan(vec3 f, float value)
{
    return vec3(
        (f.x < value) ? 1.0f : 0.0f,
        (f.y < value) ? 1.0f : 0.0f,
        (f.z < value) ? 1.0f : 0.0f);
}

vec3 LinearToSRGB(vec3 rgb)
{
    rgb = clamp(rgb, 0.0f, 1.0f);

    return mix(
        pow(rgb, vec3(1.0f / 2.4f)) * 1.055f - 0.055f,
        rgb * 12.92f,
        LessThan(rgb, 0.0031308f)
    );
}

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp(((x*(a*x+b))/(x*(c*x+d)+e)), 0.0, 1.0);
}

void main() {

    const float gamma = 2.2;
    vec3 hdrColor = texture(texSampler, outTexCoord).rgb;

//    float exposure = 0.5;
//    // exposure tone mapping
//    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

//
    vec3 mapped = ACESFilm(hdrColor.bgr);

    // gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));


    outColor = vec4(mapped.rgb, 1.0);
}