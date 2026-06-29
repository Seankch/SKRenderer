struct VSInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
};

struct SceneUBO
{
    float4x4 view;
    float4x4 proj;
};

struct GPUDrawPushConstants
{
    float4x4 model;
    float4x4 normalMat;
};

struct VSOutput
{
    float4 pos              : SV_POSITION;  // location = 0
    float2 uv               : TEXCOORD0;    // location = 1
    float3 worldPos         : TEXCOORD1;    // location = 2
    float3 worldNormal      : TEXCOORD2;    // location = 3
    float3x3 TBN            : TEXCOORD3;    // location = 4
};

[[vk::binding(0, 0)]]
ConstantBuffer<SceneUBO> uScene;

[[vk::push_constant]]
GPUDrawPushConstants pushConstants;

[shader("vertex")]
VSOutput main(VSInput _vert)
{
    // Vertex shader output
    VSOutput outVS;

    // Set clip pos
    float4x4 VP = mul(uScene.proj, uScene.view);
    outVS.pos = mul(VP, mul(pushConstants.model, float4(_vert.position, 1.0)));

    // Set out UV
    outVS.uv = float2(_vert.uv.x, _vert.uv.y);

    // Set worldPos
    float4 worldPos = mul(pushConstants.model, float4(_vert.position, 1.0));
    outVS.worldPos = worldPos.xyz;

    // Set worldNormal
    outVS.worldNormal = normalize(mul(float3x3(pushConstants.normalMat), _vert.normal));

    // Setup TBN
    float3 T = normalize(mul(pushConstants.model, float4(_vert.tangent.xyz, 0.f)).xyz);
    float3 N = normalize(mul(pushConstants.model, float4(_vert.normal, 0.f)).xyz);
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * _vert.tangent.w;
    outVS.TBN = float3x3(T, B, N);

    // Return VS output once done
    return outVS;
}
