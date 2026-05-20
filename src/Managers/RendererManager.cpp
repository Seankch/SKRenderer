/*****************************************************
    Includes
*****************************************************/
#include <Managers/RendererManager.h>

// Temp ignore unreferenced params
#pragma warning(disable: 4100)

RendererManager::RendererManager(WindowsManager* _wm) 
: windowsMgr{ _wm }
{
    // Init graphics system variables
    mMainRenderer = nullptr;
    mCurrRendererType = RendererBase::RENDERER_TYPE::RT_VULKAN; // Init to Vulkan
}

RendererManager::~RendererManager()
{
}

void RendererManager::Load()
{
    // Init renderer map
    //mGraphicsRendererMap.emplace(RendererBase::RT_OPENGL, new OpenGLRenderer());
    mGraphicsRendererMap.emplace(RendererBase::RT_VULKAN, new VulkanRenderer(windowsMgr));

    // Set main renderer
    mMainRenderer = mGraphicsRendererMap[mCurrRendererType];

    // Init framebuffers
    AddFrameBuffer("Game");

    // Initialize renderer
    mMainRenderer->Init();
}

void RendererManager::LateLoad()
{
    // Initialize late load functions
    mMainRenderer->LateInit();
}

void RendererManager::Init()
{
}

void RendererManager::Update()
{
}

void RendererManager::FixedUpdate()
{
}

void RendererManager::LateUpdate()
{
}

void RendererManager::Exit()
{
}

void RendererManager::Unload()
{
    // Free framebuffers
    for (size_t i = 0; i < mFrameBufferNames.size(); ++i)
    {
        // Free viewport FBO
        mMainRenderer->DeleteFrameBuffer(mFrameBufferNames[i]);
    }
    mFrameBufferNames.clear();

    // Delete renderers from map
    std::unordered_map<RendererBase::RENDERER_TYPE, RendererBase*>::iterator it;
    for (it = mGraphicsRendererMap.begin(); it != mGraphicsRendererMap.end(); ++it)
    {
        // Terminate renderers and delete pointers
        it->second->Exit();
        delete it->second;
        it->second = nullptr;
    }

    // Clear map
    mGraphicsRendererMap.clear();
}

void RendererManager::WaitDeviceIdle()
{
    mMainRenderer->WaitDeviceIdle();
}

void RendererManager::BeginRender(size_t _fboIdx, glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm)
{
    // Bind framebuffer and clear buffers
    mMainRenderer->BindFrameBuffer(mFrameBufferNames[_fboIdx]);
    mMainRenderer->BeginRender(_viewXForm, _projXForm);
}

void RendererManager::EndRender()
{
    // End render and unbind framebuffer
    mMainRenderer->EndRender();
    mMainRenderer->UnbindFrameBuffer();
}

RendererBase* RendererManager::GetRenderer(void)
{
    return mMainRenderer;
}

RendererBase::RENDERER_TYPE RendererManager::GetRendererType(void)
{
    return mCurrRendererType;
}

void RendererManager::CheckRendererExists(void)
{
    // Check if renderer is assigned
    if (!mMainRenderer)
    {
        // Main renderer is not assigned, exit engine.
        std::terminate();
    }
}

void RendererManager::AddFrameBuffer(std::string const& _fboName)
{
    mFrameBufferNames.emplace_back(_fboName);
    mMainRenderer->AddFrameBuffer(_fboName);
}

size_t RendererManager::GetFrameBufferCount(void)
{
    return mFrameBufferNames.size();
}
