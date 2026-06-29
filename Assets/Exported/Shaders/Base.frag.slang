struct PSInput
{
    float4 pos              : SV_Position; // location = 0 
    float2 uv               : TEXCOORD0;   // location = 1
    float3 worldPos         : TEXCOORD1;   // location = 2
    float3 worldNormal      : TEXCOORD2;   // location = 3
    float3x3 TBN            : TEXCOORD3;   // location = 4
};

struct MaterialUBO
{
    float4 UTintColor;
    int UReflective;

    // For PBR
    float URoughness;
    float UMetallic;
};

struct TextureIndexes
{
    int UMainTex;
    int UNormalMap;
};

struct GPUPointLight
{
    float4 position;
    float4 intensity;
};

struct GPUDirLight
{
    float4 direction;  // xyz = direction
    float4 intensity;  // rgb = color * intensity
};

struct GPUSceneData
{
    float4x4 view;
    float4x4 proj;
    float4 ambientColorIntensity;
    int4 lightCounts;
    GPUPointLight pointLights[MAX_POINT_LIGHTS];
    GPUDirLight   dirLights[MAX_DIR_LIGHTS];
    float3 camPos;
    uint2 renderDims;
};

struct InstanceLUT
{
    uint textureID;
    uint indexBufferOffset;
    uint uvBufferOffset;
};

struct Reservoir
{
    uint  mostImptLightIdx;
    float lightWeight;
    float sumLightWeight;
    float numLightsProcessed;
};

// Scene UBO (set=0, binding=0)
[[vk::binding(0, 0)]]
ConstantBuffer<GPUSceneData> uScene;

// TLAS (set=0, binding=1)
[[vk::binding(1, 0)]]
RaytracingAccelerationStructure accelerationStructure;

// Index buffer
[[vk::binding(2, 0)]]
StructuredBuffer<uint> indexBuffer;

// UV buffer
[[vk::binding(3, 0)]]
StructuredBuffer<float2> uvBuffer;

// LUT buffer
[[vk::binding(4, 0)]]
StructuredBuffer<InstanceLUT> instanceLUTBuffer;

// Bindless textures (set=1, binding=0)
[[vk::binding(0, 1)]]
Sampler2D textures[];

// ReSTIR resources (set=2, binding=0)
[[vk::binding(0, 2)]]
RWStructuredBuffer<Reservoir> reservoirs;

// Material UBO (set=3, binding=0)
[[vk::binding(0, 3)]]
ConstantBuffer<MaterialUBO> uMat;

// Texture Indexes (set=3, binding=1)
[[vk::binding(1, 3)]]
ConstantBuffer<TextureIndexes> uTextureIndexes;

// Max light counts
static const int MAX_POINT_LIGHTS = 64;
static const int MAX_DIR_LIGHTS   = 4;

// For ray TMin
static const float EPSILON = 1e-4f;

// For PI
static const float PI = 3.14159265358979323846;

// For getting UV of intersected pt
float2 IntersectionUV(uint instanceID, uint primIndex, float2 barycentrics) 
{
    uint indexOffset = instanceLUTBuffer[NonUniformResourceIndex(instanceID)].indexBufferOffset;

    uint i0 = indexBuffer[indexOffset + (primIndex * 3 + 0)];
    uint i1 = indexBuffer[indexOffset + (primIndex * 3 + 1)];
    uint i2 = indexBuffer[indexOffset + (primIndex * 3 + 2)];

    float2 uv0 = uvBuffer[i0];
    float2 uv1 = uvBuffer[i1];
    float2 uv2 = uvBuffer[i2];

    float w0 = 1.0 - barycentrics.x - barycentrics.y;
    float w1 = barycentrics.x;
    float w2 = barycentrics.y;

    return w0 * uv0 + w1 * uv1 + w2 * uv2;
}

// Trace shadow ray
bool TraceShadowRay(float3 origin, float3 dir, float tMax)
{
    RayDesc ray;
    ray.Origin    = origin;
    ray.Direction = dir;
    ray.TMin      = EPSILON;
    ray.TMax      = tMax;

    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> sq;
    let rayFlags = RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    sq.TraceRayInline(accelerationStructure, rayFlags, 0xFF, ray);
    
    // Loop until the ray hits an opaque object, or exceeds max range
    while(sq.Proceed())
    {
        uint instanceID = sq.CandidateRayInstanceCustomIndex();
        uint primIndex = sq.CandidatePrimitiveIndex();
        float2 uv = IntersectionUV(instanceID, primIndex, sq.CandidateTriangleBarycentrics());
        uint texID = instanceLUTBuffer[NonUniformResourceIndex(instanceID)].textureID;
        float4 intersectionColor = textures[NonUniformResourceIndex(texID)].Sample(uv);

        if (intersectionColor.a < 0.5) 
        {
            // If the triangle is transparent, we continue to trace
            // to find the next opaque triangle.
        } 
        else 
        {
            // If we hit an opaque triangle, we stop tracing.
            sq.CommitNonOpaqueTriangleHit();
        }
    }

    return (sq.CommittedStatus() == COMMITTED_TRIANGLE_HIT);
}

// Trace reflection ray
void ApplyReflection(float3 P, float3 N, inout float3 baseColor) 
{
    // Build the reflections ray
    float3 V = normalize(uScene.camPos - P);
    float3 R = reflect(-V, N);

    RayDesc reflectionRayDesc;
    reflectionRayDesc.Origin = P;
    reflectionRayDesc.Direction = R;
    reflectionRayDesc.TMin = EPSILON;
    reflectionRayDesc.TMax = 1e4;

    // Initialize a ray query for reflections
    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rq;
    let rayFlags = RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES;

    // Launch reflection ray
    rq.TraceRayInline(accelerationStructure, rayFlags, 0xFF, reflectionRayDesc);

    while (rq.Proceed())
    {
        uint instanceID = rq.CandidateRayInstanceCustomIndex();
        uint primIndex = rq.CandidatePrimitiveIndex();
        float2 uv = IntersectionUV(instanceID, primIndex, rq.CandidateTriangleBarycentrics());
        uint texID = instanceLUTBuffer[NonUniformResourceIndex(instanceID)].textureID;
        float4 intersectionColor = textures[NonUniformResourceIndex(texID)].Sample(uv);

        if (intersectionColor.a < 0.5) 
        {
            // If the triangle is transparent, we continue to trace
            // to find the next opaque triangle.
        } 
        else 
        {
            // If we hit an opaque triangle, we stop tracing.
            rq.CommitNonOpaqueTriangleHit();
        }
    }

    // Check if ray has hit
    if (rq.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        uint instanceID = rq.CommittedRayInstanceCustomIndex();
        uint primIndex = rq.CommittedPrimitiveIndex();

        float2 uv = IntersectionUV(instanceID, primIndex, rq.CommittedTriangleBarycentrics());
        uint texID = instanceLUTBuffer[NonUniformResourceIndex(instanceID)].textureID;
        float4 intersectionColor = textures[NonUniformResourceIndex(texID)].Sample(uv);
        baseColor = lerp(baseColor, intersectionColor.rgb, 0.7f);
    }
}

// PBR lighting
// GGX normal distribution function
float GGXDistribution(float nDotH)
{
    float alpha2 = uMat.URoughness * uMat.URoughness * uMat.URoughness * uMat.URoughness;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * d * d);
}

// Smith Masking function
float GeomSmith(float nDotV)
{
    float r = (uMat.URoughness + 1.0f);
    float k = (r * r) / 8.0f;
    float denom = nDotV * (1.0f - k) + k;
    return nDotV / denom;
}

// Shlick Fresnel approximation
float3 SchlickFresnel(float vDotH, float3 diffuse)
{
    float3 f0 = float3(0.04f); // Dielectrics
    if (uMat.UMetallic >= 1.0f)
        f0 = diffuse;
    return f0 + (1.0f - f0) * pow(clamp(1.0f - vDotH, 0.0f, 1.0f), 5);
}

// Compute directional light
float3 ComputeDirLight(float3 lightDir, float3 viewDir, float3 n, float3 diffuse, float3 intensity)
{
    float3 lightI = intensity;
    float3 l = lightDir;
    l = normalize(l);

    // Compute object/light vectors
    float3 v = normalize(viewDir);
    float3 h = normalize(v + l);
    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    // Compute PBR light result
    float NDF = GGXDistribution(nDotH);
    float G = GeomSmith(nDotV) * GeomSmith(nDotL);
    float3 F = SchlickFresnel(vDotH, diffuse);
    float3 num = NDF * G * F;
    float denom = max(4.0f * nDotV * nDotL, 0.0001);
    float3 specBrdf = num / denom;

    // Compute light constants
    float3 kS = F;
    float3 kD = float3(1.f) - kS;
    kD *= 1.f - uMat.UMetallic;

    return (kD * diffuse / PI + specBrdf) * lightI * max(nDotL, 0.1f);
}

// Compute point light
float3 ComputePointLight(float3 lightDir, float3 viewDir, float3 n, float3 diffuse, float att, GPUPointLight light)
{
    float3 lightI = light.intensity.rgb;
    float3 l = lightDir;

    // Set attenuation of light
    lightI *= att;

    // Compute object/light vectors
    float3 v = normalize(viewDir);
    float3 h = normalize(v + l);
    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    // Compute PBR light result
    float NDF = GGXDistribution(nDotH);
    float G = GeomSmith(nDotV) * GeomSmith(nDotL);
    float3 F = SchlickFresnel(vDotH, diffuse);
    float3 num = NDF * G * F;
    float denom = max(4.0f * nDotV * nDotL, 0.0001);
    float3 specBrdf = num / denom;

    // Compute light constants
    float3 kS = F;
    float3 kD = float3(1.f) - kS;
    kD *= 1.f - uMat.UMetallic;

    return (kD * diffuse / PI + specBrdf) * lightI * max(nDotL, 0.f);
}

[shader("fragment")]
float4 main(PSInput i) : SV_Target0
{
    // Compute normals
    float3 normal = i.worldNormal;
    if (uTextureIndexes.UNormalMap >= 0)
    {
        // Lookup normals from normal map
        // Convert to world space using TBN
        normal = textures[uTextureIndexes.UNormalMap].Sample(i.uv).rgb;
        normal = normal * 2.0f - 1.0f;
        normal = normalize(mul(i.TBN, normal));
    }

    // Compute viewdir and world pos
    float3 worldPos = i.worldPos;
    float3 viewDir = normalize(uScene.camPos - worldPos);

    // Init fragment color
    float3 texColor = (uTextureIndexes.UMainTex >= 0) ? textures[uTextureIndexes.UMainTex].Sample(i.uv).rgb : float3(1, 1, 1);
    float3 baseColor = texColor * uMat.UTintColor.rgb;
    float3 ambient = uScene.ambientColorIntensity.rgb * uScene.ambientColorIntensity.a;
    float3 finalColor = (baseColor * ambient);

    // Apply reflection
    if (uMat.UReflective == 1)
    {
        ApplyReflection(worldPos, normal, finalColor);
    }

    // Compute directional lights
    for (int i = 0; i < uScene.lightCounts.y; ++i)
    {
        // Get dir light, compute light direction
        GPUDirLight dirLight = uScene.dirLights[i];
        float3 lightDir = normalize(-dirLight.direction.xyz);
        
        // Compute shadows
        bool shadowed = TraceShadowRay(worldPos, lightDir, 1e4f);
        if (!shadowed)
        {
            // Compute directional light
            finalColor += ComputeDirLight(lightDir, viewDir, normal, baseColor, dirLight.intensity.rgb);
        }
    }

    // Compute point lights
    for (int i = 0; i < uScene.lightCounts.x; ++i)
    {
        // Get point light, compute light direction
        GPUPointLight pointLight = uScene.pointLights[i];
        float3 lightDir = pointLight.position.xyz - worldPos;

        // Compute attenuation
        float dist = length(lightDir);
        float rangeAttenuation = clamp(1.f - dist / pointLight.intensity.a, 0.f, 1.f);
        lightDir = normalize(lightDir);

        // Compute shadows
        bool shadowed = TraceShadowRay(worldPos, lightDir, rangeAttenuation);
        if (!shadowed)
        {
            // Compute point light
            finalColor += ComputePointLight(lightDir, viewDir, normal, baseColor, rangeAttenuation, pointLight);
        }
    }

    // Return final color once done
    return float4(finalColor, uMat.UTintColor.a);
}
