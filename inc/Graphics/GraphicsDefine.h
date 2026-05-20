#ifndef GRAPHICS_DEFINE_H
#define GRAPHICS_DEFINE_H

#include <string>
#include <glm/glm.hpp>

// Buffer object setup
#define POSITION_BIND_INDEX 4
#define NORMAL_BIND_INDEX 5
#define UV_BIND_INDEX 6
#define TANGENTS_BIND_INDEX 7

#define POSITION_ATTRIB_INDEX 0
#define NORMAL_ATTRIB_INDEX 1
#define UV_ATTRIB_INDEX 2
#define TANGENTS_ATTRIB_INDEX 3

#define NUMBER_ELEMENT_POS 3
#define NUMBER_ELEMENT_NORMAL 3
#define NUMBER_ELEMENT_UV 2
#define NUMBER_ELEMENT_TANGENTS 4

#define MAX_POINT_LIGHTS 64
#define MAX_DIR_LIGHTS 4

// File paths
#define ASSETS_FOLDER_PATH "Assets\\Exported\\"
#define RESOURCE_FOLDER_PATH "Assets\\Resources\\"
#define SHADER_DIR "Assets\\Resources\\Shaders\\"
#define MATERIAL_DIR "Assets\\Resources\\Materials\\"

// Set 0, 1 and 2 are reserved for scene data, bindless textures and ReSTIR resources
#define MATERIAL_MIN_INDEX 3

#endif