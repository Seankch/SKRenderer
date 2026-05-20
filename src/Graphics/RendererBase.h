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
    virtual void ClearBuffer(glm::vec4 const& _clearColor) = 0;
    virtual void Render(Model::Mesh const& _mesh, Shader& _shader, glm::vec4 const& _tint, glm::mat4 const& _modelXForm, glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm, bool _hasPreMultipliedAlpha = false) = 0;
    virtual void RenderLines(Model::Mesh const& _mesh, Shader& _shader, float _lineWidth, glm::vec4 const& _color, glm::mat4 const& _mat) = 0;
    virtual void RenderCollisionBoxes(Model::Mesh const& _mesh, Shader& _shader, glm::vec4 const& _tint, glm::mat4 const& _modelXForm, glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm, bool _hasPreMultipliedAlpha = false) = 0;
    virtual void RenderVertices(Model::Mesh const& _mesh, Shader& _shader, glm::vec4 const& _tint, glm::mat4 const& _modelXForm, glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm, bool _hasPreMultipliedAlpha = false) = 0;
    virtual void Exit() = 0;
    virtual void BeginRender() = 0;
    virtual void EndRender() = 0;

    // Shader functions
    virtual bool LoadShadersToRenderer(Shader& _shader, std::string const& _shaderName) = 0;
    virtual void UnloadShader(Shader& _shader) = 0;
    virtual void LoadShaderInfo(Shader& _shader) = 0;

    // Functions to create and render with frame buffer
    virtual void CreateFrameBuffer(std::string const& _name, bool _isFloatingPtFBO) = 0;
    virtual void DeleteFrameBuffer(std::string const& _name) = 0;
    virtual void BindFrameBuffer(std::string const& _name) = 0;
    virtual void UnbindFrameBuffer(void) = 0;
    virtual Texture& GetFrameBufferTexture(std::string const& _name) = 0;
    virtual unsigned GetFrameBufferID(std::string const& _name) = 0;
    virtual void RescaleFramebuffer(std::string const& _name, int _width, int _height, bool _lockAspectRatio, bool _isFloatingPtFBO) = 0;

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

    // Function for getting FBO size
    virtual glm::vec2 GetFBOSize(std::string const& _fboName) = 0;

    // Function for controlling depth test status
    virtual void SetDepthTestStatus(bool _isEnable) = 0;

    // For skysphere rendering
    virtual void RenderSkySphere(Model::Mesh const& _mesh, Shader& _shader) = 0;

    // Function to manage batch rendering
    //virtual unsigned CreateVAOForBatchRenderer(void) = 0;
    //virtual void CreateVBOandEBOForBatching(unsigned const& _vaoid, std::vector<float>const& _batchList, std::vector<unsigned short>& _batchIndicesList, unsigned& _vboid, unsigned& _eboid) = 0;
    //virtual void RenderBatchedObjects(unsigned const& _vaoid, WP_Matrix3x3 const& _mat, size_t _indicesSize, float _depth, WP_Shader& _shader, WP_Vector4F const& _color, WP_Texture const* _tex = nullptr) = 0;
    //virtual void FreeBatchedVBOandEBO(unsigned& _vboid, unsigned& _eboid) = 0;

    // Frame buffer variables
    struct FrameBuffer
    {
        unsigned fboid;
        unsigned rboid;
        unsigned depthbufferId;
        Texture tex;
        int fboWidth, fboHeight;
    };
    std::unordered_map<std::string, FrameBuffer> frameBufferMap;

    // Clear color
    glm::vec4 mClearColor;
};

#endif
