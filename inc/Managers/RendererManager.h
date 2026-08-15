#ifndef RENDERER_MANAGER_H
#define RENDERER_MANAGER_H

/*****************************************************
    Includes
*****************************************************/
#include <unordered_map>
#include <Managers/ManagerBase.h>

// Graphics Renderers
#include <Graphics/Vulkan/VulkanRenderer.h>

// Class for the engine's graphics system
class RendererManager : public ManagerBase
{
public:
    // Constructor/Destructor
    RendererManager(WindowsManager* _wm);
    ~RendererManager();

    // Functions for graphics system
    void Load();
    void LateLoad();
    void Init();
    void Update();
    void FixedUpdate();
    void LateUpdate();
    void Exit();
    void Unload();
    void WaitDeviceIdle();

    // RendererManager functions
    void BeginRender(size_t _fboIdx, glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm);
    void EndRender(void);

    // Function to get renderer
    RendererBase* GetRenderer(void);
    RendererBase::RENDERER_TYPE GetRendererType(void);
    void CheckRendererExists(void);

    // Function to add framebuffer to be created in graphics API backend
    void AddFrameBuffer(std::string const& _fboName);
    size_t GetFrameBufferCount(void);
private:
    // Variables for graphics renderer
    std::unordered_map<RendererBase::RENDERER_TYPE, RendererBase*> mGraphicsRendererMap;
    RendererBase* mMainRenderer;

    // Framebuffer names
    std::vector<std::string> mFrameBufferNames;

    // Current renderer type
    RendererBase::RENDERER_TYPE mCurrRendererType;

    // Required managers
    WindowsManager* windowsMgr = nullptr;
};

#endif
