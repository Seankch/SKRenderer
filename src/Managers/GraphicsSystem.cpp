#include <Managers/GraphicsSystem.h>

// Render passes
#include "Graphics/RenderGraph/RenderPasses/GBufferPass.h"

GraphicsSystem::GraphicsSystem(WindowsManager* _wm, ResourceManager* _rm, EntityManager* _em, RendererManager* _rendererMgr)
: mIsMaximized{}, mRendererType{ RT_RASTERIZE }, windowsMgr{ _wm }, resourceMgr{ _rm }, entityMgr{ _em }, rendererMgr{ _rendererMgr }
{
}

GraphicsSystem::~GraphicsSystem()
{
}

void GraphicsSystem::Load()
{
    // Setup render passes
    mRenderGraph.AddPass(new GBufferPass("GBuffer", {}, { "Position, Normals, Albedo" })); // GBuffer pass
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
    // Destroy and free render passes
    mRenderGraph.Destroy();
}

void GraphicsSystem::Render()
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

        // Create render context
        RenderContext renderContext(i, V, P, *rendererMgr, *entityMgr, *resourceMgr);

        // Execute render graph
        mRenderGraph.Execute(renderContext);
    }
}

Camera* GraphicsSystem::GetCamera(VIEWPORT_WINDOW _viewport)
{
    if (mCameras.find(_viewport) == mCameras.end()) 
    {
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
