#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include "Managers/ManagerBase.h"
#include <unordered_map>
#include <string>
#include <Graphics/GraphicsDefine.h>
#include <Graphics/Model.h>
#include <Graphics/Texture.h>
#include <Graphics/Shader.h>
#include <Graphics/Font.h>
#include <Graphics/Material.h>

// Required managers
#include <Managers/RendererManager.h>
#include <Managers/JSONManager.h>

class ResourceManager : public ManagerBase
{
public:
    // Constructor/Destructor
    ResourceManager(RendererManager* _rendererMgr, JSONManager* _jsonMgr);
    ~ResourceManager();

	// Manager Functions
    void Load();
    void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
    void LateUpdate();
	void Exit();
	void Unload();

	// For getting resource
	template<typename T>
	T* GetResource(std::string const& _name);

    // For checking if resource exist
    template<typename T>
    bool CheckResourceExist(std::string const& _name);

    // For materials
    void AddMaterial(std::string const& _matName);

    // For getting resource maps
    std::unordered_map<std::string, Model*> const& GetModelMap(void);
private:
	// Functions to add resources
	void AddModel(std::string const& _modelName, std::string const& _filePath);
	void AddShader(std::string const& _shaderName);
	void AddTexture(std::string const& _texName, std::string const& _filePath);
	void AddFont(std::string const& _fontName, std::string const& _jsonFilePath, std::string const& _textureFilePath);
	
	// Vectors for resources
	std::unordered_map<std::string, Model*> mModels;
	std::unordered_map<std::string, Shader*> mShaders;
	std::unordered_map<std::string, Texture*> mTextures;
	std::unordered_map<std::string, Font*> mFonts;
    std::unordered_map<std::string, Material*> mMaterials;

    // Required managers
    RendererManager* rendererMgr = nullptr;
    JSONManager* jsonManager = nullptr;
};

template<typename T>
inline T* ResourceManager::GetResource(std::string const& _name)
{
    if constexpr (std::is_same_v<T, Model>)
    {
        auto it = mModels.find(_name);
        return (it != mModels.end()) ? it->second : nullptr;
    }
    else if constexpr (std::is_same_v<T, Shader>)
    {
        auto it = mShaders.find(_name);
        return (it != mShaders.end()) ? it->second : nullptr;
    }
    else if constexpr (std::is_same_v<T, Texture>)
    {
        auto it = mTextures.find(_name);
        return (it != mTextures.end()) ? it->second : nullptr;
    }
    else if constexpr (std::is_same_v<T, Font>)
    {
        auto it = mFonts.find(_name);
        return (it != mFonts.end()) ? it->second : nullptr;
    }
    else if constexpr (std::is_same_v<T, Material>)
    {
        auto it = mMaterials.find(_name);
        return (it != mMaterials.end()) ? it->second : nullptr;
    }

    return nullptr;
}

template<typename T>
inline bool ResourceManager::CheckResourceExist(std::string const& _name)
{
    if constexpr (std::is_same_v<T, Model>)
    {
        auto it = mModels.find(_name);
        return (it != mModels.end());
    }
    else if constexpr (std::is_same_v<T, Shader>)
    {
        auto it = mShaders.find(_name);
        return (it != mShaders.end());
    }
    else if constexpr (std::is_same_v<T, Texture>)
    {
        auto it = mTextures.find(_name);
        return (it != mTextures.end());
    }
    else if constexpr (std::is_same_v<T, Font>)
    {
        auto it = mFonts.find(_name);
        return (it != mFonts.end());
    }
    else if constexpr (std::is_same_v<T, Material>)
    {
        auto it = mMaterials.find(_name);
        return (it != mMaterials.end());
    }

    return false;
}

#endif
