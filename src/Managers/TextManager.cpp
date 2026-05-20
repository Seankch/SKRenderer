/*!
All content © 2024 DigiPen Institute of Technology Singapore, all rights reserved.
@file       TextManager.cpp
@author     Sean Kwek Chin Huat
@co-author

----------------------------------------------------------------------------

@course     CSD3401F24
@project    CSD3401F24 Software Engineering Project 5

@brief      This file contains definitions for functions to handle
						rendering of fonts.
*//*______________________________________________________________________*/

/*****************************************************
		Includes
*****************************************************/
#include <Managers/TextManager.h>

// Temporarily here to access assets (to be replaced with AssetManager)
#include <Managers/GraphicsSystem.h>

void TextManager::Init(void)
{
}

void TextManager::Load()
{
}

void TextManager::Update()
{
	// Allow text billboarding
	auto& reg = EntityManager::GetInstance()->mECS;
	for (auto& ent : reg.view<entt::entity>())
	{
		// Check if entity has Text component
		if (!reg.all_of<Transform, Text>(ent)) {
			// Entity does not have Text, skip
			continue;
		}

		// Entity has transform & text component, perform billboarding if needed
		Transform& xform = reg.get<Transform>(ent);
		Text& text = reg.get<Text>(ent);

		// Check if text is billboarded
		if (text.GetIsTextFaceCamera())
		{
			// Compute dir from text to cam
			glm::vec3 camPos = GraphicsSystem::GetInstance()->GetCamera(GraphicsSystem::VW_GAME)->GetPosition();
			glm::vec3 dir = glm::normalize(camPos - xform.GetPos());

			// Compute pitch and yaw
			float yaw = atan2(dir.x, dir.z);
			float pitch = asin(dir.y);

			// Set pitch and yaw
			xform.SetRot(glm::degrees(glm::vec3(-pitch, yaw, 0.0f)));
		}
	}
}

void TextManager::FixedUpdate()
{
}

void TextManager::Free()
{
}

void TextManager::Unload()
{
}

void TextManager::Render(glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm)
{
	// Fetch default font shader
	// mDefaultFontShader = AssetManager::GetInstance()->GetShader("FontShader");
	mDefaultFontShader = GraphicsSystem::GetInstance()->mShaders["FontShader"];
	if (!mDefaultFontShader)
		return;

	// Fetch quad model
	// mDefaultFontShader = AssetManager::GetInstance()->GetModel("Quad");
	Model* quadModel = ResourceManager::GetInstance()->GetResource<Model>(GraphicsSystem::GetInstance()->mModels["Quad"]);
	if (!quadModel)
		return;

	// Get projection xform matrix
	glm::mat4 P = _projXForm;
	glm::mat4 defaultViewXForm = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1.f, 1 };

	// Get current font
	// Add texts for render
	auto& reg = EntityManager::GetInstance()->mECS;
	for (auto& ent : reg.view<entt::entity>())
	{
		// Check if entity has text/transform component
		if (!reg.all_of<Transform, Text>(ent))
		{
			// Entity does not have text/transform, skip
			continue;
		}

		// Entity has transform & text component, begin render
		Transform& xform = reg.get<Transform>(ent);
		Text& text = reg.get<Text>(ent);
		Font* currentFont = GraphicsSystem::GetInstance()->mFonts[text.GetFontName()];
		if (!currentFont)
			continue;

		if (!text.IsRendering())
			return;

		// Get view xform matrix
		glm::mat4 V = (text.IsInGameText()) ? _viewXForm : defaultViewXForm;

		// Compute total width of each glyph
		float totalWidth = 0.f;
		std::string textStr = text.GetTextString();
		for (size_t j = 0; textStr[j] != '\0'; ++j)
		{
			// Get glyph associated with char
			Font::GlyphData glyph = currentFont->GetGlyphData(textStr[j]);

			// Compute total width of word
			totalWidth += glyph.uvScale.x * xform.GetScale().x;
			if (textStr[j + 1] != '\0')
			{
				// Add spacing between each word
				totalWidth += text.GetTextSpacing();
			}
		}

		// Set text start position
		float startPosX{}, originalPosX, newlineOffset{};
		if (text.GetTextStyle() == Text::TS_FIRST_CHAR_AT_POS)
		{
			// Start rendering from the given position
			startPosX = xform.GetPos().x;
			originalPosX = startPosX;
		}
		else
		{
			// Center the text
			startPosX = xform.GetPos().x - (totalWidth * 0.5f);
		}

		// Begin rendering the text
		for (int j = 0; textStr[j] != '\0'; ++j)
		{
			// Get glyph associated with char
			Font::GlyphData glyph = currentFont->GetGlyphData(textStr[j]);

			// Handle newlines
			if (textStr[j] == '\n')
			{
				// Reset x position
				if (text.GetTextStyle() == Text::TS_FIRST_CHAR_AT_POS)
				{
					// Start rendering from the given position
					startPosX = xform.GetPos().x;
					originalPosX = startPosX;
				}
				else
				{
					// Center the text
					startPosX = xform.GetPos().x - (totalWidth * 0.5f);
				}

				// Move to next line
				newlineOffset -= text.GetNewLineOffset();
				continue;
			}
			// Handle spaces
			else if (textStr[j] == '_')
			{
				// Move to next char's position
				startPosX += (glyph.uvScale.x * xform.GetScale().x * 0.5f) + text.GetTextSpacing();
				continue;
			}

			// Compute glyph render position
			float xPos = startPosX + (glyph.uvScale.x * xform.GetScale().x * 0.5f);
			float glyphHeight = glyph.planeMax.y - glyph.planeMin.y;
			float glyphCenter = glyph.planeMin.y + (glyphHeight * 0.5f);
			float yPos = xform.GetPos().y + glyphCenter + newlineOffset;
			glm::vec3 renderPos{ xPos, yPos, xform.GetPos().z };

			// Compute render scale
			float scaleFactor = 8.f;
			float xScale = glyph.uvScale.x;
			float yScale = glyph.uvScale.y * scaleFactor;
			glm::vec3 textScale{ xScale, yScale, 1.f };

			// Set UV scale and offset
			mDefaultFontShader->SetUniform("uvScale", glyph.uvScale * 0.99f);
			mDefaultFontShader->SetUniform("uvOffset", glyph.uvOffset);

			// Form MVP transform matrices
			glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, renderPos.x, renderPos.y, renderPos.z, 1 };
			glm::mat4 textScaleMat = { textScale.x * xform.GetScale().x, 0, 0, 0, 0, textScale.y, 0, 0, 0, 0, textScale.z, 0, 0, 0, 0, 1 };
			glm::mat4 renderScaleMat = { scaleFactor, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
			glm::mat4 M = renderScaleMat * translateMat * textScaleMat;

			// Handle text rotation
			glm::vec3 rotRadians = glm::radians(xform.GetRot());
			glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
			glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
			glm::mat4 rotMat = rotMatZ * rotMatY * rotMatX;
			glm::mat4 worldXFormMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1};
			glm::mat4 invWorldXformMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -xform.GetPos().x, -xform.GetPos().y, -xform.GetPos().z, 1 };
			M = worldXFormMat * rotMat * invWorldXformMat * M;

			// Render glyph
			mDefaultFontShader->SetUniformTexture("UMainTex", currentFont->GetTexture()->textureID);
			RendererManager::GetInstance()->Render(quadModel->mMeshes.front(), *mDefaultFontShader, text.GetTextColor(), M, V, P, false);

			// Move to next char's position
			startPosX += (glyph.uvScale.x * xform.GetScale().x) + text.GetTextSpacing();
		}
	}
}

TextManager::TextManager()
{
}
