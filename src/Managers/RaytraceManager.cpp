#include "Managers/RaytraceManager.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"

RaytraceManager::RaytraceManager(EntityManager* _entityMgr, ResourceManager* _resourceMgr, RendererManager* _rendererMgr)
{
    entityMgr = _entityMgr;
    resourceMgr = _resourceMgr;
    rendererMgr = _rendererMgr;
}

RaytraceManager::~RaytraceManager()
{
}

void RaytraceManager::Load()
{
    CreateAccelerationStructures();
    CreateReSTIRResources();
}

void RaytraceManager::LateLoad()
{
    CreateLUTResources();
}

void RaytraceManager::Init()
{
}

void RaytraceManager::Update()
{
    // Get main renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();

    // Update instances to use new transform matrices
    entt::registry& ecs = entityMgr->GetECS();
    for (auto& ent : ecs.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!ecs.all_of<Transform, MeshRenderer>(ent))
        {
            // Entity does not have Transform/MeshRenderer, skip
            continue;
        }

        // If mesh is not rendering, skip
        MeshRenderer& mr = ecs.get<MeshRenderer>(ent);
        Model* model = resourceMgr->GetResource<Model>(mr.GetModel());
        if (!mr.IsRendering())
        {
            for (int meshIdx = 0; meshIdx < model->mMeshes.size(); ++meshIdx)
            {
                glm::mat4 scaleMat = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
                mainRenderer->UpdateBLASInstanceTransform(static_cast<uint32_t>(ent), meshIdx, scaleMat);
            }

            continue;
        }

        // Set transform matrix
        Transform& xform = ecs.get<Transform>(ent);
        glm::vec3 rotRadians = glm::radians(xform.GetRot());
        glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
        glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
        glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
        glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
        glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

        // Update instance transform matrix
        for (int meshIdx = 0; meshIdx < model->mMeshes.size(); ++meshIdx)
        {
            mainRenderer->UpdateBLASInstanceTransform(static_cast<uint32_t>(ent), meshIdx, M * model->mMeshes[meshIdx].initialXFormMat);
        }
    }

    // Update TLAS
    mainRenderer->UpdateTLAS();
}

void RaytraceManager::FixedUpdate()
{
}

void RaytraceManager::LateUpdate()
{
}

void RaytraceManager::Exit()
{
}

void RaytraceManager::Unload()
{
    // Destroy ray tracing resources
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    mainRenderer->DestroyRaytracingResources();
    mainRenderer->DestroyLUTResources();
    mainRenderer->DestroyReSTIRResources();
}

void RaytraceManager::CreateAccelerationStructures(void)
{
    // Create BLAS for each mesh
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    mainRenderer->CreateBLAS(resourceMgr->GetModelMap());

    // Create BLAS instances for each entity
    entt::registry& ecs = entityMgr->GetECS();
    for (auto& ent : ecs.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!ecs.all_of<Transform, MeshRenderer>(ent))
        {
            // Entity does not have Transform/MeshRenderer, skip
            continue;
        }

        // Set transform matrix
        Transform& xform = ecs.get<Transform>(ent);
        glm::vec3 rotRadians = glm::radians(xform.GetRot());
        glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
        glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
        glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
        glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
        glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

        // Create BLAS instance for entity
        MeshRenderer& mr = ecs.get<MeshRenderer>(ent);
        Model* model = resourceMgr->GetResource<Model>(mr.GetModel());
        mainRenderer->CreateBLASInstances(static_cast<uint32_t>(ent), *model, M);
    }

    // Create TLAS
    mainRenderer->CreateTLAS();
}

void RaytraceManager::CreateLUTResources(void)
{
    // Get main renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    
    // Add instances to LUT list
    entt::registry& ecs = entityMgr->GetECS();
    for (auto& ent : ecs.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!ecs.all_of<Transform, MeshRenderer>(ent))
        {
            // Entity does not have Transform/MeshRenderer, skip
            continue;
        }

        // Get mesh renderer
        MeshRenderer& mr = ecs.get<MeshRenderer>(ent);

        // Get mesh renderer's model
        Model* mdl = resourceMgr->GetResource<Model>(mr.GetModel());
        if (!mdl)
            continue;

        // For each submesh, add instance to LUT list
        for (size_t i = 0; i < mdl->mMeshes.size(); ++i)
        {
            // Get texture ID
            Material* mat = resourceMgr->GetResource<Material>(mr.GetMaterialList()[i]);
            std::optional<TextureEntry> tex = mat->GetUniform<TextureEntry>("UMainTex");

            if (!tex.has_value())
            {
                // If shader doesn't have UMainTex, stop
                std::terminate();
            }

            // Add instance to LUT list
            mainRenderer->AddInstanceToLUTList(tex.value().ID, *mdl, static_cast<uint32_t>(i));
        }
    }

    // Create LUT resources (buffers, etc)
    mainRenderer->CreateLUTResources(resourceMgr->GetModelMap());
}

void RaytraceManager::CreateReSTIRResources(void)
{
    // Get main renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();

    // Create ReSTIR buffers
    mainRenderer->CreateReSTIRResources();
}
