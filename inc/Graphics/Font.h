#ifndef FONT_H
#define FONT_H

/*****************************************************
	Includes
*****************************************************/
#include <unordered_map>
#include <Graphics/Texture.h>
#include <Graphics/GraphicsDefine.h>

class Font
{
public:
	// Struct to store glyph data
	struct GlyphData
	{
		glm::vec2 uvOffset;
		glm::vec2 uvScale;
		glm::vec2 planeMin;
		glm::vec2 planeMax;
		glm::vec2 atlasMin;
		glm::vec2 atlasMax;
		float advance;
	};

	// Function to load fonts
	bool LoadFontFamily(const char* _fileDir);

	// Functions to get font info
	int GetTotalAtlasWidth(void);
	int GetTotalAtlasHeight(void);
	GlyphData& GetGlyphData(char _c);

	// Function to get/set texture
	void SetTexture(Texture* _tex);
	Texture* GetTexture(void);
private:
	// ID for font texture atlas
	Texture* mTexture = nullptr;

	// Store total texture atlas width/height
	int mTotalAtlasWidth{};
	int mTotalAtlasHeight{};

	// Map containing characters loaded from font
	std::unordered_map<char, GlyphData> mGlyphDataMap;
};

#endif