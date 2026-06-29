#include "Managers/AssetManager.h"
#include "Graphics/GraphicsDefine.h"
#include <filesystem>

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

void AssetManager::Load()
{
    // Map of all compile functions
    std::unordered_map<std::string, std::function<void(std::filesystem::path const&)>> assetCompileFunctionList
    {
        { "Models",    [this](std::filesystem::path const& _p) { CompileMeshes(_p.string()); }},   // Models
        { "Textures",  [this](std::filesystem::path const& _p) { CompileTextures(_p.string()); }}, // Textures
        { "Shaders",   [this](std::filesystem::path const& _p) { CompileShaders(_p.string()); }},  // Shaders
    };

    // Iterate through all folders in asset folder, compile all assets
    std::filesystem::directory_entry matFolder{};
    for (std::filesystem::directory_entry const& folder : std::filesystem::directory_iterator(ASSETS_FOLDER_PATH))
    {
        // Get resource type
        std::string resTypeStr = folder.path().stem().string();

        // Check if folder exist, and if it's a directory
        if (std::filesystem::exists(folder.path()) && std::filesystem::is_directory(folder.path()))
        {
            // Iterate through all files in folder, load all resources
            for (std::filesystem::directory_entry const& file : std::filesystem::recursive_directory_iterator(folder.path()))
            {
                // Call respective Add function
                assetCompileFunctionList[resTypeStr](file.path());
            }
        }
    }
}

void AssetManager::LateLoad()
{
}

void AssetManager::Init()
{
}

void AssetManager::Update()
{
}

void AssetManager::FixedUpdate()
{
}

void AssetManager::LateUpdate()
{
}

void AssetManager::Exit()
{
}

void AssetManager::Unload()
{
}

void AssetManager::CompileMeshes(std::string const& _filePath)
{
}

void AssetManager::CompileTextures(std::string const& _filePath)
{
}

void AssetManager::CompileShaders(std::string const& _filePath)
{
    // Compile shader
    shaderCompiler.Compile(_filePath);
}
