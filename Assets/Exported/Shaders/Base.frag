#version 450

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

// Material UBO
layout(set = 1, binding = 0) uniform MaterialUBO
{
    vec4 TintColor;
} uMat;

// Main texture
layout (set = 1, binding = 1) uniform sampler2D UMainTex;

void main()
{
    // Get texture and set color
    vec3 texColor = texture(UMainTex, inUV).rgb;
    vec3 color = texColor * uMat.TintColor.rgb;

    // Set out color with tintColor alpha
    outFragColor = vec4(color, uMat.TintColor.a);
}
