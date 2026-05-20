#ifndef IMAGE_H
#define IMAGE_H

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Model.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <unordered_map>

// Class to encapsulate UI rendering data
class Image 
{
public:
    Image();
    ~Image();

    // Getters
    std::string const& GetTexture(void);
    std::string const& GetShader(void);
    const glm::vec4& GetColor(void);
    bool IsRendering(void);
    bool HasPremultipliedAlpha(void);

    // Setters
    void SetTexture(std::string const& _texName);
    void SetShader(std::string const& _shaderName);
    void SetColor(glm::vec4 const& _color);
    void SetIsRendering(bool _isRendering);
    void SetHasPremultipliedAlpha(bool _hasPremultipliedAlpha);
private:
    // Variables for image
    std::string mShaderName;
    std::string mTextureName;
    bool mIsRendering{};
    bool mHasPremultipliedAlpha{};

    // Variables and functions for setting color
    glm::vec4 mColor{};
};

#endif