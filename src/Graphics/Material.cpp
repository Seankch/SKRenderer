/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Material.h>
#include <Managers/GraphicsSystem.h>
//#include <rttr/registration>
#include <DIContainer/DIContainer.h>

Material::Material() : mShaderName{}, mCullMode{ CM_CULL_NONE }, mIsPremultipliedAlpha{}
{
}

Material::~Material()
{
}

std::string const& Material::GetName(void) const
{
    return mMatName;
}

void Material::SetName(std::string const& _name)
{
    mMatName = _name;
}

std::string const& Material::GetShader(void) const
{
    return mShaderName;
}

void Material::SetUniformValue(std::string const& _uniform, VariantType const& _t)
{
    // If uniform doesn't exist in map, don't add it
    if (mUniformsMap.find(_uniform) == mUniformsMap.end())
        return;

    // Set uniform value
    mUniformsMap[_uniform] = _t;
}

std::unordered_map<std::string, VariantType>& Material::GetUniformMap(void)
{
    return mUniformsMap;
}

void Material::SetShader(Shader const& _shader)
{
    // If same name, no need to set again
    if (mShaderName == _shader.shaderName)
        return;

    // Set shader name
    mShaderName = _shader.shaderName;

    // Add shader uniforms variables to map
    PopulateMap(_shader);
}

void Material::PopulateMap(Shader const& _shader)
{
    // Clear map first
    mUniformsMap.clear();

    // Add uniform variables to map
    std::vector<Uniform> const& shaderUniformList = _shader.GetUniformList();
    for (size_t i = 0; i < shaderUniformList.size(); ++i)
    {
        Uniform const& u = shaderUniformList[i];
        if (u.type == UT_FLOAT_VEC2)
        {
            mUniformsMap.emplace(u.uniformName, glm::vec2{});
        }
        else if (u.type == UT_FLOAT_VEC3)
        {
            mUniformsMap.emplace(u.uniformName, glm::vec3{});
        }
        else if (u.type == UT_FLOAT_VEC4)
        {
            mUniformsMap.emplace(u.uniformName, glm::vec4{});
        }
        else if (u.type == UT_FLOAT)
        {
            mUniformsMap.emplace(u.uniformName, float{});
        }
        else if (u.type == UT_DOUBLE)
        {
            mUniformsMap.emplace(u.uniformName, double{});
        }
        else if (u.type == UT_INT)
        {
            mUniformsMap.emplace(u.uniformName, int{});
        }
        else if (u.type == UT_UNSIGNED_INT)
        {
            mUniformsMap.emplace(u.uniformName, GLuint{});
        }
        else if ((u.type == UniformType::UT_TEXTURE_INDEX) || (u.type == UniformType::UT_SAMPLER2D))
        {
            mUniformsMap.emplace(u.uniformName, TextureEntry{});
        }
    }
}

bool Material::GetIsPremultipliedAlpha(void)
{
    return mIsPremultipliedAlpha;
}

void Material::SetIsPremultipliedAlpha(bool _isPremultipliedAlpha)
{
    mIsPremultipliedAlpha = _isPremultipliedAlpha;
}

Material::CULL_MODE Material::GetCullMode(void) const
{
    return mCullMode;
}

void Material::SetCullMode(CULL_MODE _mode)
{
    mCullMode = _mode;
}

//RTTR_REGISTRATION
//{
//    rttr::registration::class_<Material>("Material")
//        .property("Material Name", &Material::GetName, &Material::SetName)
//        .property("Shader Name", &Material::GetShader, &Material::SetShader)
//        .property("Tint Color", &Material::GetTintColor, &Material::SetTintColor)
//        .property("Cull Mode", &Material::GetCullMode, &Material::SetCullMode)
//        .property("Is premultiplied alpha", &Material::GetIsPremultipliedAlpha, &Material::SetIsPremultipliedAlpha);
//
//    rttr::registration::enumeration<Material::CULL_MODE>("Cull Mode")
//    (
//        rttr::value("Cull Front", Material::CM_CULL_FRONT),
//        rttr::value("Cull Back", Material::CM_CULL_BACK),
//        rttr::value("Cull Font&Back", Material::CM_CULL_FRONT_BACK),
//        rttr::value("Cull None", Material::CM_CULL_NONE)
//    );
//}
