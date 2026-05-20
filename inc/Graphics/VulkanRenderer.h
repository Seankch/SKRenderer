#ifndef VULKAN_RENDERER
#define VULKAN_RENDERER

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/RendererBase.h>
#include <Graphics/VulkanPipeline.h>
#include <Graphics/VulkanDescriptorPool.h>
#include <Graphics/VulkanDescriptor.h>
#include <Graphics/VulkanReSTIRHandler.h>
#include "vulkan/vulkan.h"
#include <optional>
#include <functional>
#include <deque>
#include "Vulkan/Include/vma/vk_mem_alloc.h"
#include "Graphics/VulkanTypes.h"
#include "Graphics/VulkanShader.h"
#include "Graphics/VulkanLoader.h"

// Required manager includes
#include "Managers/WindowsManager.h"

#define MAX_FRAMES_IN_FLIGHT 2

class VulkanRenderer : public RendererBase
{
public:
    VulkanRenderer(WindowsManager* _wm);
    ~VulkanRenderer();
    void Init(void);
    void LateInit(void);
    void ClearBuffer(void);
    void Render(Model::Mesh const& _mesh, Material& _mat, glm::mat4 const& _modelXForm, bool _hasPreMultipliedAlpha = false, bool _isFirstObject = false);
    void Exit(void);
    void BeginRender(glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm);
    void EndRender(void);
    void WaitDeviceIdle(void);

    // Shader functions
    bool LoadShadersToRenderer(Shader& _shader, std::string const& _shaderName);
    void UnloadShader(Shader& _shader);
    void LoadShaderInfo(Shader& _shader);

    // Material functions
    void LoadMaterialToRenderer(Material& _mat);
    void LoadUniformsToShader(Material const& _mat, Shader const& _shader);
    void FreeMaterial(std::string const& _matName);
    
    template<class T>
    void SetUniformToShader(void* _mappedData, size_t _offset, T const& _val);

    // Functions to manage frame buffer
    void AddFrameBuffer(std::string const& _fboName);
    void CreateFrameBuffer(std::string const& _fboName, bool _isFloatingPtFBO);
    void DeleteFrameBuffer(std::string const& _name);
    void BindFrameBuffer(std::string const& _name);
    void UnbindFrameBuffer(void);
    Texture& GetFrameBufferTexture(std::string const& _name);
    unsigned GetFrameBufferID(std::string const& _name);
    void RescaleFrameBuffer(std::string const& _name, int _width, int _height, bool _lockAspectRatio = false, bool _isFloatingPtFBO = false);

    // Functions to init and free model VAO
    void LoadMeshToRenderer(Model::Mesh& _mesh);
    void FreeMesh(Model::Mesh& _mesh);

    // Functions to init and free texture from renderer
    void LoadTextureToRenderer(Texture& _texture);
    void FreeTexture(Texture& _texture);

    // Functions to set render mode
    void SetRenderMode(RENDER_MODE _mode);

    // Function to set cull mode
    void SetCullStatus(bool _enable);
    void SetCullMode(Material::CULL_MODE _mode);
    void ResetCullMode(void);

    // Read/write to texture
    uint32_t ReadPixel(std::string const& _fboName, int _x, int _y);

    // Function for controlling depth test status
    void SetDepthTestStatus(bool _isEnable);

    // For rendering skysphere
    void RenderSkySphere(Model::Mesh const& _mesh, Shader& _shader);

    // For setting lighting variables
    void SetAmbientLight(glm::vec3 const& _color, float _intensity);
    void SetPointLight(glm::vec3 const& _worldPos, glm::vec3 const& _intensity, float _range, int _index);
    void SetDirectionalLight(glm::vec3 const& _dir, glm::vec3 const& _intensity, int _index);
    void SetLightCounts(int _pointLightCount, int _dirLightCount);

    // For passing camera uniforms to shader
    void SetCameraPosition(glm::vec3 const& _camPos);

    // For initializing and rendering imgui for vulkan
    void InitImGUI();
    void RenderImGUI();

    // For raytracing
    void CreateBLAS(std::unordered_map<std::string, Model*> const& _modelMap);
    void CreateTLAS(void);
    void CreateBLASInstances(uint32_t _entityID, Model const& _model, glm::mat4 const& _xform);
    void UpdateBLASInstanceTransform(uint32_t _entityID, int _meshIdx, glm::mat4 const& _xform);
    void UpdateTLAS(void);
    void DestroyRaytracingResources(void);
    void AddInstanceToLUTList(int _texID, Model const& _model, uint32_t _meshIdx);
    void CreateLUTResources(std::unordered_map<std::string, Model*> const& _modelMap);
    void DestroyLUTResources(void);

    // For ReSTIR
    void CreateReSTIRResources(void);
    void DestroyReSTIRResources(void);
    void UpdateReSTIRResources(void);
private:
    // For deletion of vulkan structs
    struct DeletionQueue
    {
        std::deque<std::function<void()>> deleteList;

        void AddToList(std::function<void()>&& _func)
        {
            deleteList.push_back(_func);
        }

        void Flush()
        {
            for (auto it = deleteList.rbegin(); it != deleteList.rend(); it++)
            {
                (*it)();
            }

            deleteList.clear();
        }
    };

    // For queue families
    struct QueueFamilyIndices
    {
        // Store different queue families
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        // Check if all queue families are supported
        bool IsComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // For querying swap chain support
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};    // Min/max number of images in swap chain, min/max width height of images
        std::vector<VkSurfaceFormatKHR> formats;    // Surface formats (pixel format, color space)
        std::vector<VkPresentModeKHR> presentModes; // Available presentation modes
    };

    // For controlling per frame data
    struct FrameData
    {
        // For deletion of frame data
        DeletionQueue deleteQueue;

        // Command pool and buffer
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        // Fences and semaphores
        VkSemaphore presentSemaphore;
        VkFence renderFence;

        // Frame Descriptors
        VulkanDescriptorPool frameDescriptors;

        // Scene data buffer
        AllocatedBuffer sceneDataBuffer;
    };

    struct VulkanFrameBuffer
    {
        AllocatedImage drawImg;
        AllocatedImage depthImg;
        VkExtent2D extent;
        bool isFloatingPtFBO;
    };

    struct BLASData
    {
        VkAccelerationStructureKHR blasHandle;
        AllocatedBuffer blasBuffer;
        AllocatedBuffer scratchBuffer;
    };

    struct TLASData
    {
        VkAccelerationStructureKHR tlasHandle{};
        AllocatedBuffer tlasBuffer{};
        AllocatedBuffer scratchBuffer{};
        AllocatedBuffer instanceBuffer{};
        std::unordered_map<uint32_t, std::unordered_map<int, int>> instanceIDMap;
    };

    struct LUTResources
    {
        AllocatedBuffer indexBuffer{};
        AllocatedBuffer uvBuffer{};
        AllocatedBuffer lutBuffer{};
        size_t indexBufferSize{};
        size_t uvBufferSize{};
        size_t lutBufferSize{};
    };

    // Instance creation
    void CreateInstance(void);
    bool CheckValidationLayerSupport(void);

    // Surface creation
    void CreateSurface(void);

    // Picking physical devices
    void PickPhysicalDevice(void);
    int RateDeviceSuitability(VkPhysicalDevice const& _device);

    // For finding queue families
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice const& _device);

    // For logical devices
    void CreateLogicalDevice(void);

    // For device extensions
    bool CheckDeviceExtensionSupport(VkPhysicalDevice const& _device);

    // For swap chain
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice const& _device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& _formats);
    VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& _presentModes);
    VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR const& _capabilities);
    void CreateSwapChain(void);
    void DestroySwapChain(void);
    void RecreateSwapChain(void);

    // For image views
    void CreateImageViews(void);

    // Graphics pipeline
    void CreateGraphicsPipeline(Material const& _mat);
    void DestroyGraphicsPipeline(void);

    // For loading/storing shaders
    std::vector<uint32_t> ReadShaderFile(std::string const& _fileName);
    VkShaderModule* CreateShaderModule(std::vector<uint32_t> const& _shaderCode);
    std::unordered_map<std::string, VulkanShader> mShaderMap;

    // For command pool and buffer
    void CreateCommandPoolAndBuffer(void);

    // For synchronization
    void CreateSyncObjects(void);

    // For frames
    FrameData& GetCurrFrame(void);

    // For memory allocator
    void InitMemoryAllocator(void);

    // For framebuffers
    void InitFrameBuffers(void);

    // For descriptors
    void InitDescriptorPools(void);
    void InitDescriptors(void);

    // For immediate submit
    void ImmediateSubmit(std::function<void(VkCommandBuffer const& _cmd)>&& _func);

    // Functions to create vk pipelines
    VulkanPipeline& CreatePipeline(Material const& _mat);

    // For Texture loading/rendering
    std::vector<AllocatedImage> mTextureList;
    void InitDefaultSamplers(void);

    // Variables for renderer
    VkInstance mInst;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mLogicalDevice;
    VkSurfaceKHR mSurface;
    VkSwapchainKHR mSwapChain;
    bool mEnableValidationLayers;
    uint32_t mMinUboOffsetAlignment;

    // For loading extension functions
    VulkanLoader mVkLoader;

    // Variables for queue families
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    // Variables for swap chain images
    std::vector<VkImage> mSwapChainImages;
    VkFormat mSwapChainImageFormat;
    VkExtent2D mSwapChainExtent;
    std::vector<VkSemaphore> mRenderSemaphoreList;

    // Variables for image views
    std::vector<VkImageView> mSwapChainImageViews;

    // Variables for frame
    uint32_t mFrameIndex{};
    FrameData mFrames[MAX_FRAMES_IN_FLIGHT]{};

    // Descriptor pool for materials, draw image, bindless textures, etc
    VulkanDescriptorPool mDescriptorPool{};

    // Variables for draw image descriptors
    VkDescriptorSet mDrawImageDescriptors{};
    VkDescriptorSetLayout mDrawImageDescriptorLayout{};

    // Containers
    std::vector<char const*> mValidationLayers;
    std::vector<char const*> mDeviceExtensions;
    std::vector<VkDynamicState> mDynamicStates;

    // For deletion of vulkan renderer structs
    DeletionQueue mMainDeleteQueue;

    // Memory allocator
    VmaAllocator mAllocator{};

    // For immediate submit
    VkFence mImmediateFence{};
    VkCommandPool mImmediateCmdPool{};
    VkCommandBuffer mImmediateCmdBuffer{};

    // Used for rendering
    uint32_t mSwapChainImageIndex{};
    AllocatedImage* mDrawImg{};
    AllocatedImage* mDepthImg{};
    VkExtent2D* mDrawExtent{};
    VkDescriptorSet mGlobalDescriptor{};
    VkDescriptorSet mBindlessTextureDescriptor{};
    std::unordered_map <std::string, VulkanFrameBuffer> mFrameBuffers;
    std::unordered_map <std::string, VulkanPipeline> mPipelineMap;
    std::unordered_map <std::string, VulkanMaterialData> mMatMap;

    // Per frame descriptors to handle scene data
    GPUSceneData sceneData{};

    // Global descriptor set layouts
    VkDescriptorSetLayout mGpuSceneDataDescLayout{};
    VkDescriptorSetLayout mBindlessTextureDescLayout{};

    // Variables for samplers
    VkSampler mDefaultSamplerLinear{};

    // For ray tracing
    std::unordered_map<std::string, std::vector<unsigned>> blasIDList;
    std::vector<VkAccelerationStructureInstanceKHR> blasInstanceList;
    std::vector<BLASData> blasDataList;
    TLASData tlasData;

    // For LUT (look up table)
    LUTResources mLUTResources{};

    // For ReSTIR
    VulkanReSTIRHandler mReSTIRHandler{};

    // Required managers
    WindowsManager* windowsMgr = nullptr;
};

template<class T>
inline void VulkanRenderer::SetUniformToShader(void* _mappedData, size_t _offset, T const& _val)
{
    // Offset the pointer by the binding's offset
    std::byte* dataStart = static_cast<std::byte*>(_mappedData) + _offset;

    // Memcpy the data with sizeof(T)
    size_t size = sizeof(T);
    memcpy(dataStart, &_val, size);
}

#endif
