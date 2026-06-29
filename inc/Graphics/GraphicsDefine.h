#ifndef GRAPHICS_DEFINE_H
#define GRAPHICS_DEFINE_H

#include <string>
#include <glm/glm.hpp>

#define MAX_POINT_LIGHTS 64
#define MAX_DIR_LIGHTS 4

// File paths
#define ASSETS_FOLDER_PATH "..\\..\\Assets\\Exported\\"
#define RESOURCE_FOLDER_PATH "..\\..\\Assets\\Resources\\"
#define SHADER_DIR "..\\..\\Assets\\Resources\\Shaders\\"
#define MATERIAL_DIR "..\\..\\Assets\\Resources\\Materials\\"
#define TOOLS_DIR "..\\..\\Tools\\"

// Compiler defines
#define SHADER_COMPILER "ShaderCompiler\\slangc.exe"

// Set 0, 1 and 2 are reserved for scene data, bindless textures and ReSTIR resources
#define MATERIAL_MIN_INDEX 3

#endif