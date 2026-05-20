#ifndef TEXTURE_H
#define TEXTURE_H

#include <Graphics/GraphicsDefine.h>

// Texture constants
#define FOURCC_DXT1       0x31545844
#define FOURCC_DXT3       0x33545844
#define FOURCC_DXT5       0x35545844
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

class Texture
{
public:
	bool LoadFromFile(const char* filePath);
	void UnloadPixelInformation();
	int width{};
	int height{};
	int numComponents{};
	int mipMapCount{};
	unsigned char* textureData{};
	int format{};
	int textureID{};
};

#endif