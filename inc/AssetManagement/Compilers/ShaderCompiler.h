#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H

/*****************************************************
    Includes
*****************************************************/
#include "AssetCompilerBase.h"

class ShaderCompiler : public AssetCompilerBase
{
public:
    void Compile(std::string const& _filePath);

private:
    struct ShaderStage
    {
        std::string shaderStage;    // Shader stage name
        std::string shaderStageExt; // Shader stage file extension
    };

    ShaderStage ParseShaderStage(std::string const& _filePath);
};

#endif