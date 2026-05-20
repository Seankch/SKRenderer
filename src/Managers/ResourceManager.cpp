#include "Managers/ResourceManager.h"
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <fstream>

#pragma warning(push, 0)
#include <prettywriter.h>
#include <ostreamwrapper.h>
#include <istreamwrapper.h>
#pragma warning(pop)

ResourceManager::ResourceManager(RendererManager* _rendererMgr, JSONManager* _jsonMgr)
: rendererMgr{ _rendererMgr }, jsonManager{ _jsonMgr }
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Load()
{
    // Map of all Add functions
    std::unordered_map<std::string, std::function<void(std::filesystem::path const&)>> resLoadFunctionList
    {
        { "Models",    [this](std::filesystem::path const& _p) { AddModel(_p.stem().string(), _p.string()); }},   // Models
        { "Shaders",   [this](std::filesystem::path const& _p) { AddShader(_p.stem().stem().string()); }},        // Shaders
        { "Textures",  [this](std::filesystem::path const& _p) { AddTexture(_p.stem().string(), _p.string()); }}, // Textures
    };

    // Iterate through all folders in resource folder, load everything except for materials
    for (std::filesystem::directory_entry const& folder : std::filesystem::directory_iterator(RESOURCE_FOLDER_PATH))
    {
        // Get resource type
        std::string resTypeStr = folder.path().stem().string();

        // If resoure type is not in map, skip
        if (resLoadFunctionList.find(resTypeStr) == resLoadFunctionList.end())
        {
            continue;
        }

        // Check if folder exist, and if it's a directory
        if (std::filesystem::exists(folder.path()) && std::filesystem::is_directory(folder.path()))
        {
            // Iterate through all files in folder, load all resources
            for (std::filesystem::directory_entry const& file : std::filesystem::recursive_directory_iterator(folder.path()))
            {
                // Call respective Add function
                resLoadFunctionList[resTypeStr](file.path());
            }
        }
    }
}

void ResourceManager::LateLoad()
{
    // Load materials
    std::filesystem::path materialsPath = std::filesystem::path(RESOURCE_FOLDER_PATH) / "Materials";
    if (std::filesystem::exists(materialsPath))
    {
        for (std::filesystem::directory_entry const& file : std::filesystem::recursive_directory_iterator(materialsPath))
        {
            AddMaterial(file.path().stem().string());
        }
    }
}

void ResourceManager::Init()
{
}

void ResourceManager::Update()
{
}

void ResourceManager::FixedUpdate()
{
}

void ResourceManager::LateUpdate()
{
}

void ResourceManager::Exit()
{
}

void ResourceManager::Unload()
{
    // Get main renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();

    // Free model
    std::unordered_map<std::string, Model*>::iterator mdlItr;
    for (mdlItr = mModels.begin(); mdlItr != mModels.end(); ++mdlItr)
    {
        // Free meshes first
        Model* model = mdlItr->second;
        for (int i = 0; i < model->mMeshes.size(); ++i)
        {
            mainRenderer->FreeMesh(model->mMeshes[i]);
        }

        // Delete model
        delete model;
        model = nullptr;
    }

    // Free shader
    std::unordered_map<std::string, Shader*>::iterator shaderItr;
    for (shaderItr = mShaders.begin(); shaderItr != mShaders.end(); ++shaderItr)
    {
        Shader* shader = shaderItr->second;
        mainRenderer->UnloadShader(*shader);
        delete shader;
        shader = nullptr;
    }

    // Free texture
    std::unordered_map<std::string, Texture*>::iterator texItr;
    for (texItr = mTextures.begin(); texItr != mTextures.end(); ++texItr)
    {
        Texture* tex = texItr->second;
        mainRenderer->FreeTexture(*tex);
        tex->UnloadPixelInformation();
        delete tex;
        tex = nullptr;
    }

    // Free font
    std::unordered_map<std::string, Font*>::iterator fontItr;
    for (fontItr = mFonts.begin(); fontItr != mFonts.end(); ++fontItr)
    {
        Font* font = fontItr->second;
        delete font->GetTexture();
        delete font;
        font = nullptr;
    }

    // Free material
    std::unordered_map<std::string, Material*>::iterator matItr;
    for (matItr = mMaterials.begin(); matItr != mMaterials.end(); ++matItr)
    {
        Material* mat = matItr->second;
        mainRenderer->FreeMaterial(mat->GetName());
        delete mat;
        mat = nullptr;
    }

    // Clear maps
    mModels.clear();
    mShaders.clear();
    mTextures.clear();
    mFonts.clear();
    mMaterials.clear();
}

// Functions to add resources
void ResourceManager::AddModel(std::string const& _modelName, std::string const& _filePath)
{
    // If model already exists, don't add
    if (mModels.find(_modelName) != mModels.end())
        return;

    // Begin loading model
    Model* model = new Model();
    if (!model->LoadModelFile(_filePath))
    {
        std::cout << "Unable to create: " << _modelName << " model!" << std::endl;
        delete model;
        return;
    }

    // Set name
    model->modelName = _modelName;
    std::cout << "New Model: " << _modelName << " Added!" << std::endl;

    // Load meshes to renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    for (int i = 0; i < model->mMeshes.size(); ++i)
    {
        mainRenderer->LoadMeshToRenderer(model->mMeshes[i]);
    }

    // Add model to map
    mModels.emplace(_modelName, model);
}

void ResourceManager::AddShader(std::string const& _shaderName)
{
    // If shader already exists, don't add
    if (mShaders.find(_shaderName) != mShaders.end())
        return;

    // Add to shader map
    Shader* shader = new Shader();
    mShaders.emplace(_shaderName, shader);

    // Set base variables
    shader->shaderName = _shaderName;

    // Load shaders to renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    mainRenderer->LoadShadersToRenderer(*shader, _shaderName);

    // Load shader bindings
    mainRenderer->LoadShaderInfo(*shader);
}

void ResourceManager::AddTexture(std::string const& _texName, std::string const& _filePath)
{
    // If texture is already in map, no need to make new texture
    if (mTextures.find(_texName) != mTextures.end())
        return;

    Texture* texture = new Texture();
    if (!texture->LoadFromFile(_filePath.c_str()))
    {
        std::cout << "Unable to create: " << _texName << " texture!" << std::endl;
        delete texture;
        return;
    }

    // Load texture to renderer
    RendererBase* mainRenderer = rendererMgr->GetRenderer();
    mainRenderer->LoadTextureToRenderer(*texture);

    // Add to list
    mTextures.emplace(_texName, texture);
    std::cout << "New Texture: " << _texName << " Added!" << std::endl;
}

void ResourceManager::AddFont(std::string const& _fontName, std::string const& _jsonFilePath, std::string const& _textureFilePath)
{
    // If font is already in map, no need to make new texture
    if (mFonts.find(_fontName) != mFonts.end())
        return;

    Font* font = new Font();
    if (!font->LoadFontFamily(_jsonFilePath.c_str()))
    {
        delete font;
        return;
    }

    // Load texture atlas
    Texture* fontTextureAtlas = new Texture{};
    fontTextureAtlas->LoadFromFile(_textureFilePath.c_str());
    font->SetTexture(fontTextureAtlas);

    // Add to map
    mFonts.emplace(_fontName, font);
}

void ResourceManager::AddMaterial(std::string const& _matName)
{
    // If material already exists, don't add
    if (mMaterials.find(_matName) != mMaterials.end())
        return;

    // Open material file
    std::ifstream matFile{};
    matFile.open(MATERIAL_DIR + _matName + ".material");
    if (!matFile.good())
    {
        return;
    }

    // Load json file
    rapidjson::IStreamWrapper stream(matFile);
    rapidjson::Document jsonFile{};
    jsonFile.ParseStream(stream);
    jsonManager->SetCurrentFile(&jsonFile);
    matFile.close();

    // Load material properties
    Material* mat = new Material();
    mat->SetName(jsonManager->Get<std::string>("MatName"));
    mat->SetShader(*GetResource<Shader>(jsonManager->Get<std::string>("ShaderName")));
    mat->SetCullMode(static_cast<Material::CULL_MODE>(jsonManager->Get<int>("CullMode")));
    mat->SetIsPremultipliedAlpha(jsonManager->Get<bool>("IsPremultipliedAlpha"));

    // If material failed to load, exit
    if (mat->GetName().empty())
        return;

    // If shader is empty, don't load uniforms
    if (mat->GetShader().empty())
        return;

    // Fetch shader properties
    Shader* shader = GetResource<Shader>(mat->GetShader());
    if (!shader)
        return;
    std::vector<Uniform> const& uList = shader->GetUniformList();

    // Set material properties
    for (size_t i = 0; i < uList.size(); ++i)
    {
        Uniform const& u = uList[i];
        if (u.type == UniformType::UT_FLOAT_VEC2)
        {
            glm::vec2 temp = jsonManager->Get<glm::vec2>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_FLOAT_VEC3)
        {
            glm::vec3 temp = jsonManager->Get<glm::vec3>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_FLOAT_VEC4)
        {
            glm::vec4 temp = jsonManager->Get<glm::vec4>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_FLOAT)
        {
            float temp = jsonManager->Get<float>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_DOUBLE)
        {
            double temp = jsonManager->Get<double>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_INT)
        {
            int temp = jsonManager->Get<int>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if (u.type == UniformType::UT_UNSIGNED_INT)
        {
            unsigned temp = jsonManager->Get<GLuint>(uList[i].uniformName);
            mat->SetUniformValue(uList[i].uniformName, temp);
        }
        else if ((u.type == UniformType::UT_SAMPLER2D) || (u.type == UniformType::UT_TEXTURE_INDEX))
        {
            std::string texName = jsonManager->Get<std::string>(uList[i].uniformName);
            int texID = -1;
            if (mTextures.find(texName) != mTextures.end())
            {
                // Get texture ID
                texID = mTextures[texName]->textureID;
            }
            
            mat->SetUniformValue(uList[i].uniformName, TextureEntry{ texName, texID });
        }
    }

    // Add to map
    mMaterials.emplace(mat->GetName(), mat);

    // Close file once done
    jsonManager->CloseFile();

    // Create material pipeline
    RendererBase* currRenderer = rendererMgr->GetRenderer();
    currRenderer->LoadMaterialToRenderer(*mat);
}

std::unordered_map<std::string, Model*> const& ResourceManager::GetModelMap(void)
{
    return mModels;
}
