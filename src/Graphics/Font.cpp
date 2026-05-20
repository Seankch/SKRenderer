/*****************************************************
	Includes
*****************************************************/
#include <Graphics/Font.h>

// For parsing json chunk
#pragma warning(push, 0)
#include <document.h>
#include <filereadstream.h>
#include <error/en.h>
#pragma warning(pop)

bool Font::LoadFontFamily(const char* _fileDir)
{
	// Load font atlas json info
    FILE* file = fopen(_fileDir, "rb");
    if (!file)
    {
        // Failed to open file, exit
        return false;
    }

    // Create a read buffer and parse the JSON
    char* readBuffer = new char[65536];
    rapidjson::FileReadStream frs(file, readBuffer, sizeof(readBuffer));
    rapidjson::Document document;
    document.ParseStream(frs);

    // Close file once done
    delete[] readBuffer;
    fclose(file);

    if (document.HasParseError()) {
        return false; // Error while parsing
    }

    // Parse the atlas data
    rapidjson::Value const& atlas = document["atlas"];
    mTotalAtlasWidth = atlas["width"].GetInt();
    mTotalAtlasHeight = atlas["height"].GetInt();

    // Load glyph data from atlas
    rapidjson::Value const& glyphs = document["glyphs"];
    for (rapidjson::SizeType i = 0; i < glyphs.Size(); i++) 
    {
        // Get current character
        rapidjson::Value const& glyph = glyphs[i];
        char ch = static_cast<char>(glyph["unicode"].GetInt());

        // Load glyph data info
        GlyphData glyphData;
        if (glyph.HasMember("planeBounds"))
        {
            rapidjson::Value const& planeBounds = glyph["planeBounds"];

            // Set plane min/max
            glyphData.planeMin = glm::vec2(planeBounds["left"].GetFloat(), planeBounds["bottom"].GetFloat());
            glyphData.planeMax = glm::vec2(planeBounds["right"].GetFloat(), planeBounds["top"].GetFloat());
        }

        // Parse atlasBounds (texcoords of chars on the atlas)
        if (glyph.HasMember("atlasBounds"))
        {
            rapidjson::Value const& atlasBounds = glyph["atlasBounds"];

            // Set atlas min/max
            glyphData.atlasMin = glm::vec2(atlasBounds["left"].GetFloat() / mTotalAtlasWidth, 1.f - (atlasBounds["top"].GetFloat() / mTotalAtlasHeight));
            glyphData.atlasMax = glm::vec2(atlasBounds["right"].GetFloat() / mTotalAtlasWidth, 1.f - (atlasBounds["bottom"].GetFloat() / mTotalAtlasHeight));

            // Compute UV offset and scale
            glyphData.uvOffset = glyphData.atlasMin;
            glyphData.uvScale = glyphData.atlasMax - glyphData.atlasMin;
        }

        // Set advance
        glyphData.advance = glyph["advance"].GetFloat() / mTotalAtlasWidth;

        // Store glyphData to map
        mGlyphDataMap[ch] = glyphData;
    }

    return true;
}

int Font::GetTotalAtlasWidth(void)
{
	return mTotalAtlasWidth;
}

int Font::GetTotalAtlasHeight(void)
{
    return mTotalAtlasHeight;
}

Font::GlyphData& Font::GetGlyphData(char _c)
{
	return mGlyphDataMap[_c];
}

void Font::SetTexture(Texture* _tex)
{
	mTexture = _tex;
}

Texture* Font::GetTexture(void)
{
	return mTexture;
}
