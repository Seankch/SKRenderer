/*!
All content © 2024 DigiPen Institute of Technology Singapore, all rights reserved.
@file       RendererBase.h
@author     Sean KWEK Chin Huat
@co-author

----------------------------------------------------------------------------

@course     CSD3401F24
@project    CSD3401F24 Software Engineering Project 5

@brief      This header file contains the base class to encapsulate a graphics renderer.
*//*______________________________________________________________________*/

#ifndef RENDERER_BASE
#define RENDERER_BASE

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Model.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Graphics/Material.h>
#include <unordered_map>

class RendererBase
{
public:
    enum RENDER_MODE
    {
        RM_LINES,
        RM_FILLED
    };
    enum RENDERER_TYPE 
    {
        RT_NONE,
        RT_OPENGL,
        RT_VULKAN,
    };

    // Base renderer functions
    virtual ~RendererBase() {};
    virtual void Init(void) = 0;
    virtual void LateInit(void) = 0;
    virtual void ClearBuffer(void) = 0;
    virtual void Render(Model::Mesh const& _mesh, Material& _mat, glm::mat4 const& _modelXForm, bool _hasPreMultipliedAlpha = false) = 0;
    virtual void Exit(void) = 0;
    virtual void BeginRender(glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm) = 0;
    virtual void EndRender(void) = 0;
    virtual void WaitDeviceIdle(void) = 0;

    // Shader functions
    virtual bool LoadShadersToRenderer(Shader& _shader, std::string const& _shaderName) = 0;
    virtual void UnloadShader(Shader& _shader) = 0;
    virtual void LoadShaderInfo(Shader& _shader) = 0;

    // Material functions
    virtual void LoadMaterialToRenderer(Material& _mat) = 0;
    virtual void LoadUniformsToShader(Material const& _mat, Shader const& _shader) = 0;
    virtual void FreeMaterial(std::string const& _matName) = 0;

    // Functions to create and render with frame buffer
    virtual void AddFrameBuffer(std::string const& _fboName) = 0; // Add framebuffer to be created
    virtual void CreateFrameBuffer(std::string const& _fboName, bool _isFloatingPtFBO) = 0;
    virtual void DeleteFrameBuffer(std::string const& _name) = 0;
    virtual void BindFrameBuffer(std::string const& _name) = 0;
    virtual void UnbindFrameBuffer(void) = 0;
    virtual Texture& GetFrameBufferTexture(std::string const& _name) = 0;
    virtual unsigned GetFrameBufferID(std::string const& _name) = 0;
    virtual void RescaleFrameBuffer(std::string const& _name, int _width, int _height, bool _lockAspectRatio, bool _isFloatingPtFBO) = 0;

    // Functions to init and free model VAO
    virtual void LoadMeshToRenderer(Model::Mesh& _mesh) = 0;
    virtual void FreeMesh(Model::Mesh& _mesh) = 0;

    // Functions to init and free texture from renderer
    virtual void LoadTextureToRenderer(Texture& _texture) = 0;
    virtual void FreeTexture(Texture& _texture) = 0;

    // Function to set render mode
    virtual void SetRenderMode(RENDER_MODE _mode) = 0;

    // Function to set cull mode
    virtual void SetCullStatus(bool _enable) = 0;
    virtual void SetCullMode(Material::CULL_MODE _mode) = 0;
    virtual void ResetCullMode(void) = 0;

    // Functions for reading/writing to texture
    virtual uint32_t ReadPixel(std::string const& _fboName, int _x, int _y) = 0;

    // Function for controlling depth test status
    virtual void SetDepthTestStatus(bool _isEnable) = 0;

    // For skysphere rendering
    virtual void RenderSkySphere(Model::Mesh const& _mesh, Shader& _shader) = 0;

    // For setting lighting variables
    virtual void SetAmbientLight(glm::vec3 const& _color, float _intensity) = 0;
    virtual void SetPointLight(glm::vec3 const& _worldPos, glm::vec3 const& _intensity, float _range, int _index) = 0;
    virtual void SetDirectionalLight(glm::vec3 const& _dir, glm::vec3 const& _intensity, int _index) = 0;
    virtual void SetLightCounts(int _pointLightCount, int _dirLightCount) = 0;
    
    // For setting camera uniforms
    virtual void SetCameraPosition(glm::vec3 const& _camPos) = 0;

    // For raytracing
    virtual void CreateBLAS(std::unordered_map<std::string, Model*> const& _modelMap) = 0;
    virtual void CreateTLAS(void) = 0;
    virtual void CreateBLASInstances(uint32_t _entityID, Model const& _model, glm::mat4 const& _xform) = 0;
    virtual void UpdateBLASInstanceTransform(uint32_t _entityID, int _meshIdx, glm::mat4 const& _xform) = 0;
    virtual void UpdateTLAS(void) = 0;
    virtual void DestroyRaytracingResources(void) = 0;
    virtual void AddInstanceToLUTList(int _texID, Model const& _model, uint32_t _meshIdx) = 0;
    virtual void CreateLUTResources(std::unordered_map<std::string, Model*> const& _modelMap) = 0;
    virtual void DestroyLUTResources(void) = 0;

    // For ReSTIR
    virtual void CreateReSTIRResources(void) = 0;
    virtual void DestroyReSTIRResources(void) = 0;
    virtual void UpdateReSTIRResources(void) = 0;

    // Frame buffer variables
    struct FrameBuffer
    {
        Texture tex{};
        unsigned fboid{};
        unsigned rboid{};
        unsigned depthbufferId{};
        int fboWidth{}, fboHeight{};
    };
    std::unordered_map<std::string, FrameBuffer> frameBufferMap;

    struct LUTInstance
    {
        int textureID;
        uint32_t indexBufferOffset;
        uint32_t uvBufferOffset;
    };
    std::vector<LUTInstance> lutList;
    std::unordered_map<std::string, std::vector<std::vector<uint32_t>>> lutListCreateHelper;

    // Clear color
    glm::vec4 mClearColor{ 0.1f, 0.1f, 0.1f, 1.f };
};

#endif
