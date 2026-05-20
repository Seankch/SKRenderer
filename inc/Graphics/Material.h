#ifndef MATERIAL_H
#define MATERIAL_H

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/GraphicsDefine.h>
#include <Graphics/Shader.h>
#include <unordered_map>
#include <variant>
#include <optional>
#include <string>

// For multitexture support
struct TextureEntry
{
    // Name and texture ID
    std::string mTextureName{};
    int ID{};
};

// Declare viable uniform variable types
using VariantType = std::variant<glm::vec2, glm::vec3, glm::vec4, float, double, int, unsigned, TextureEntry>;

// Class to encapsulate rendering data
class Material 
{
public:
    Material();
    ~Material();

    // Getters/Setters
    std::string const& GetName(void) const;
    void SetName(std::string const& _name);
    std::string const& GetShader(void) const;
    void SetShader(Shader const& _shader);

    // Functions for setting uniform value
    void SetUniformValue(std::string const& _uniform, VariantType const& _t);

    // Used to fetch uniform value
    template<typename T>
    std::optional<T> GetUniform(std::string const& _uniformName) const;

    // Get uniforms map for rttr
    std::unordered_map<std::string, VariantType>& GetUniformMap(void);

    // For culling
    enum CULL_MODE
    {
        CM_CULL_FRONT,
        CM_CULL_BACK,
        CM_CULL_FRONT_BACK,
        CM_CULL_NONE
    };

    // Temp for handling blend modes (will update to enums in M4)
    bool GetIsPremultipliedAlpha(void);
    void SetIsPremultipliedAlpha(bool _isPremultipliedAlpha);

    // Culling getter/setter
    CULL_MODE GetCullMode(void) const;
    void SetCullMode(CULL_MODE _mode);
private:
    // Material name
    std::string mMatName;

    // Ref to shader
    std::string mShaderName;

    // For culling
    CULL_MODE mCullMode;

    // Containers for uniform variables
    std::unordered_map<std::string, VariantType> mUniformsMap;

    // For blend modes
    bool mIsPremultipliedAlpha;

    // Add uniforms to map
    void PopulateMap(Shader const& _shader);
};

template<typename T>
std::optional<T> Material::GetUniform(std::string const& _uniformName) const
{
    // If uniform doesn't exists in map, display error message
    if (mUniformsMap.find(_uniformName) == mUniformsMap.end())
    {
        // Display error message
        return std::nullopt;
    }

    // Return value corresponding to uniform name
    return std::get<T>(mUniformsMap.at(_uniformName));
}

#endif