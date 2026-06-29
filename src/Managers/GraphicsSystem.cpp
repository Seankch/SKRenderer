#include <Managers/GraphicsSystem.h>

GraphicsSystem::GraphicsSystem(WindowsManager* _wm,
                               ResourceManager* _rm,
                               EntityManager* _em,
                               RendererManager* _rendererMgr)
: mIsMaximized{}, mRendererType{ RT_RASTERIZE }, windowsMgr{ _wm }, resourceMgr{ _rm }, entityMgr{ _em }, rendererMgr{ _rendererMgr }
{
}

GraphicsSystem::~GraphicsSystem()
{
}

void GraphicsSystem::Load()
{
}

void GraphicsSystem::LateLoad()
{
}

void GraphicsSystem::Init()
{
    // Set main camera
    auto& reg = entityMgr->GetECS();
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has Camera component
        if (!reg.all_of<Camera>(ent))
        {
            // Entity does not have Camera, skip
            continue;
        }

        // Entity has Camera component, check if main camera
        Camera& cam = reg.get<Camera>(ent);
        if (cam.GetIsMainCam())
        {
            // Set main camera to game viewport
            mCameras[VIEWPORT_WINDOW::VW_GAME] = &cam;
            break;
        }
    }
}

void GraphicsSystem::Update()
{
}

void GraphicsSystem::FixedUpdate()
{

}

void GraphicsSystem::LateUpdate()
{
}

void GraphicsSystem::Exit()
{
}

void GraphicsSystem::Unload()
{
}

void GraphicsSystem::Render()
{
    if (mRendererType == RT_RASTERIZE)
    {
        RenderRasterize();
    }
    else if (mRendererType == RT_RAYTRACE)
    {
        RenderRayTrace();
    }
}

void GraphicsSystem::RenderRasterize()
{
    // Check if renderer exists
    rendererMgr->CheckRendererExists();

    // Get main renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();

    // Render onto framebuffers
    size_t numFBO = rendererMgr->GetFrameBufferCount();
    for (size_t i = 0; i < numFBO; ++i)
    {
        // Get current viewport
        VIEWPORT_WINDOW currViewport = static_cast<VIEWPORT_WINDOW>(i);

        // Get view and projection transform from camera
        glm::mat4 V = mCameras[currViewport]->GetViewXForm();
        glm::mat4 P = mCameras[currViewport]->GetProjXForm();

        // Pass camera uniforms to shader
        mainRenderer->SetCameraPosition(mCameras[currViewport]->GetPosition());

        // Bind and clear frame buffer
        rendererMgr->BeginRender(i, V, P);

        // Enable culling
        mainRenderer->SetCullStatus(true);

        // Render all game objects
        auto& reg = entityMgr->GetECS();
        for (auto& ent : reg.view<entt::entity>())
        {
            // Check if entity has MeshRenderer component
            if (!reg.all_of<Transform, MeshRenderer>(ent))
            {
                // Entity does not have MeshRenderer, skip
                continue;
            }

            // Entity has transform & MeshRenderer component, begin render
            Transform& xform = reg.get<Transform>(ent);
            MeshRenderer& mr = reg.get<MeshRenderer>(ent);
            // Check if mesh is rendering
            if (!mr.IsRendering())
            {
                // If not rendering, skip
                continue;
            }

            // Compute model transform matrix
            glm::vec3 rotRadians = glm::radians(xform.GetRot());
            glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
            glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
            glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
            glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
            glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
            glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

            // Prepare variables for render
            Model* model = resourceMgr->GetResource<Model>(mr.GetModel());
            if (!model)
                continue;

            // Store time (similar to _Time in HLSL)
            float time = static_cast<float>(windowsMgr->mElapsedTime);
            glm::vec4 shaderTime{ time / 20.f, time, time * 2.f, time * 3.f };

            // Render GO
            for (size_t j = 0; j < model->mMeshes.size(); ++j)
            {
                // Get mesh's material instance
                Material* matInstance = mr.GetMatInstance(static_cast<int>(j));
                if (!matInstance || matInstance->GetName().empty()) 
                {
                    // Fall back to default material
                    matInstance = resourceMgr->GetResource<Material>(mr.GetMaterialList()[j]);

                    // If default material does not exists, don't render
                    if (!matInstance || matInstance->GetName().empty())
                    {
                        continue;
                    }
                }

                // Check if material has a valid shader
                if (matInstance->GetShader().empty())
                {
                    continue;
                }

                // Set base uniform variables
                Shader& shader = *resourceMgr->GetResource<Shader>(matInstance->GetShader());

                // Load other uniforms to shader
                mainRenderer->LoadUniformsToShader(*matInstance, shader);

                // Render all meshes in model
                mainRenderer->SetCullMode(matInstance->GetCullMode());
                mainRenderer->Render(model->mMeshes[j], *matInstance, M * model->mMeshes[j].initialXFormMat, matInstance->GetIsPremultipliedAlpha());
                mainRenderer->ResetCullMode();
            }
        }

        // Disable culling
        mainRenderer->SetCullStatus(false);

        // Unbind frame buffer
        rendererMgr->EndRender();
    }
}

void GraphicsSystem::RenderRayTrace()
{

}

Camera* GraphicsSystem::GetCamera(VIEWPORT_WINDOW _viewport)
{
    if (mCameras.find(_viewport) == mCameras.end()) {
        return nullptr;
    }

    // Camera exists, return camera
    return mCameras[_viewport];
}

void GraphicsSystem::SetCamera(VIEWPORT_WINDOW _viewport, Camera* _cam)
{
    mCameras[_viewport] = _cam;
}

void GraphicsSystem::SetIsMaximized(bool _isMaximized)
{
    mIsMaximized = _isMaximized;
}

bool GraphicsSystem::GetIsMaximized(void)
{
    return mIsMaximized;
}
