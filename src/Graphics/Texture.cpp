/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Texture.h>
#include <Managers/RendererManager.h>
#include <fstream>
#include <iostream>

// For stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

bool Texture::LoadFromFile(const char* filePath)
{
    // Load image using stb_image
	textureData = stbi_load(filePath, &width, &height, &numComponents, STBI_rgb_alpha);
	if (!textureData)
		return false;

	// Set format
	format = VK_FORMAT_R8G8B8A8_UNORM;

	// Return true if texture successfully loaded
	return true;
}
	 
void Texture::UnloadPixelInformation()
{
    delete[] textureData;
	textureData = nullptr;
}
