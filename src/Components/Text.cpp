#include <Components/Text.h>
#include <rttr/registration>
#include "EngineUI/Properties.h"

Text::Text() : mTextString{},
mFontName{},
mTextColor{ 0.f, 0.f, 0.f, 1.f },
mTextSpacing{ 0.05f },
mIsRendering{ true },
mIsInGameText{ false },
mTextStyle{ TS_CENTERED },
mNewLineOffset{ 40.f },
mIsFaceCamera{ false }
{
}

Text::~Text()
{
}

std::string const& Text::GetTextString() const
{
	return mTextString;
}

std::string const& Text::GetFontName() const
{
	return mFontName;
}

glm::vec4 const& Text::GetTextColor() const
{
	return mTextColor;
}

float Text::GetTextSpacing() const
{
	return mTextSpacing;
}

bool Text::IsRendering() const
{
	return mIsRendering;
}

bool Text::IsInGameText() const
{
	return mIsInGameText;
}

Text::TEXT_STYLE Text::GetTextStyle() const
{
	return mTextStyle;
}

float Text::GetNewLineOffset() const
{
	return mNewLineOffset;
}

bool Text::GetIsTextFaceCamera() const
{
	return mIsFaceCamera;
}

void Text::SetTextString(std::string const& _textString)
{
	mTextString = _textString;
}

void Text::SetFontName(std::string const& _fontName)
{
	mFontName = _fontName;
}

void Text::SetTextColor(glm::vec4 const& _textColor)
{
	mTextColor = _textColor;
}

void Text::SetTextSpacing(float _textSpacing)
{
	mTextSpacing = _textSpacing;
}

void Text::SetRendering(bool _isRendering)
{
	mIsRendering = _isRendering;
}

void Text::SetIsInGameText(bool _isInGameText)
{
	mIsInGameText = _isInGameText;
}

void Text::SetTextStyle(TEXT_STYLE _textStyle)
{
	mTextStyle = _textStyle;
}

void Text::SetNewLineOffset(float _newLineOffset)
{
	mNewLineOffset = _newLineOffset;
}

void Text::SetIsTextFaceCamera(bool _isFaceCamera)
{
	mIsFaceCamera = _isFaceCamera;
}

RTTR_REGISTRATION
{
		rttr::registration::class_<Text>("Text")
				.property("Text", &Text::GetTextString, &Text::SetTextString)
				.property("Font Name", &Text::GetFontName, &Text::SetFontName)
				.property("Text Color", &Text::GetTextColor, &Text::SetTextColor)
				(
					rttr::metadata(META_COLOR, true)
				)
				.property("Spacing", &Text::GetTextSpacing, &Text::SetTextSpacing)
				.property("Rendering?", &Text::IsRendering, &Text::SetRendering)
				.property("In Game Text?", &Text::IsInGameText, &Text::SetIsInGameText)
				.property("Style", &Text::GetTextStyle, &Text::SetTextStyle)
				.property("Offset", &Text::GetNewLineOffset, &Text::SetNewLineOffset)
				.property("Enable Billboard", &Text::GetIsTextFaceCamera, &Text::SetIsTextFaceCamera);
}