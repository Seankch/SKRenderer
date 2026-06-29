#include "AssetManagement/Compilers/ShaderCompiler.h"
#include "Graphics/GraphicsDefine.h"
#include <stdexcept>
#include <filesystem>
#include "iostream"

void ShaderCompiler::Compile(std::string const& _filePath)
{
    // Parse shader name
    std::filesystem::path path(_filePath);
    std::string shaderName = path.stem().stem().string();

    // Parse shader stage
    ShaderStage shaderStage = ParseShaderStage(_filePath);

    // Create filepaths for compiler tool, input shader file, output compiled shader file
    std::string shaderCompilerPath = std::filesystem::absolute(static_cast<std::string>(TOOLS_DIR) + SHADER_COMPILER).string();
    std::string inputShaderPath = std::filesystem::absolute(_filePath).string();
    std::string outputShaderPath = std::filesystem::absolute(SHADER_DIR + shaderName + "." + shaderStage.shaderStageExt + ".spv").string();

    // Append " to handle spaces in file path
    shaderCompilerPath = "\"" + shaderCompilerPath + "\"";
    inputShaderPath = "\"" + inputShaderPath + "\"";
    outputShaderPath = "\"" + outputShaderPath + "\"";

    // Create executable command for slangc
    std::string entryFunc = std::string("-entry ") + "main"; // Entry function is always "main" for now
    std::string shaderStageCmd = std::string("-stage ") + shaderStage.shaderStage;
    std::string outputDir = std::string("-o ") + outputShaderPath; // Compile to SPIR-V for now
    std::string exeCmd = shaderCompilerPath + " " + inputShaderPath + " " + entryFunc + " " + shaderStageCmd + " " + outputDir;

    // Run executable to compile shader
    RunExecutable(exeCmd);

    // Print success message
    std::cout << "Compiled shader: " << shaderName << " (" << shaderStage.shaderStage << ")" << std::endl;
}

ShaderCompiler::ShaderStage ShaderCompiler::ParseShaderStage(std::string const& _filePath)
{
    // Return shader stage based on shader file extension
    if (_filePath.find(".vert") != std::string::npos)
    {
        return ShaderStage{ "vertex", "vert"};
    }
    else if (_filePath.find(".frag") != std::string::npos)
    {
        return ShaderStage{ "fragment", "frag" };
    }
    else if (_filePath.find(".comp") != std::string::npos)
    {
        return ShaderStage{ "compute", "comp" };
    }

    // Invalid shader stage, throw error
    throw std::runtime_error("Invalid shader stage! " + _filePath);
}
