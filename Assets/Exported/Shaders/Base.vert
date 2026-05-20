#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outWorldPos;
layout (location = 2) out vec3 outWorldNormal;
layout (location = 3) out mat3 TBN;

struct Vertex {
    vec3 position;
    float uvX;
    vec3 normal;
    float uvY;
    vec4 tangent;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// GPU scene data
layout(set = 0, binding = 0) uniform SceneUBO
{
    mat4 view;
    mat4 proj;
} uScene;

layout(push_constant) uniform constants
{
    mat4 model;
    VertexBuffer vertexBuffer;
} PushConstants;

void main()
{
    // Set gl_Position
    Vertex vert = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
    mat4 VP = uScene.proj * uScene.view;
    gl_Position = VP * PushConstants.model * vec4(vert.position, 1.0);

    // Set out uv
    outUV = vec2(vert.uvX, vert.uvY);

    // Set out worldpos
    vec4 worldPos = PushConstants.model * vec4(vert.position, 1.0);
    outWorldPos = worldPos.xyz;

    // Set out world normal
    mat3 normalMat = transpose(inverse(mat3(PushConstants.model)));
    outWorldNormal = normalize(normalMat * vert.normal);

    // Setup TBN
    vec3 T = normalize(vec3(PushConstants.model * vec4(vert.tangent.xyz, 0.f)));
    vec3 N = normalize(vec3(PushConstants.model * vec4(vert.normal, 0.f)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * vert.tangent.w;
    TBN = mat3(T, B, N);
}
