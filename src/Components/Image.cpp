/*!
All content © 2024 DigiPen Institute of Technology Singapore, all rights reserved.
@file       Image.cpp
@author     Sean KWEK Chin Huat
@co-author

----------------------------------------------------------------------------

@course     CSD3401F24
@project    CSD3401F24 Software Engineering Project 5

@brief      This file contains definitions for functions to store UI properties.
*//*______________________________________________________________________*/

#include <Components/Image.h>
#include <rttr/registration>
#include "EngineUI/Properties.h"

Image::Image()
{
	mShaderName = {};
	mTextureName = {};
	mIsRendering = true;

	// Init base color
	mColor = { 1.f, 1.1f, 1.f, 1.f };
}

Image::~Image()
{
}

std::string const& Image::GetTexture(void)
{
	return mTextureName;
}

std::string const& Image::GetShader(void)
{
	return mShaderName;
}

const glm::vec4& Image::GetColor(void)
{
	return mColor;
}

bool Image::IsRendering(void)
{
	return mIsRendering;
}

bool Image::HasPremultipliedAlpha(void)
{
	return mHasPremultipliedAlpha;
}

void Image::SetTexture(std::string const& _texName)
{
	mTextureName = _texName;
}

void Image::SetShader(std::string const& _shaderName)
{
	mShaderName = _shaderName;
}

void Image::SetColor(glm::vec4 const& _color)
{
	mColor = _color;
}

void Image::SetIsRendering(bool _isRendering)
{
	mIsRendering = _isRendering;
}

void Image::SetHasPremultipliedAlpha(bool _hasPremultipliedAlpha)
{
	mHasPremultipliedAlpha = _hasPremultipliedAlpha;
}

RTTR_REGISTRATION
{
	rttr::registration::class_<Image>("Material")
	.constructor<>()
	.property("Texture", &Image::GetTexture, &Image::SetTexture)
	.property("Shader", &Image::GetShader, &Image::SetShader)
	.property("Color", &Image::GetColor, &Image::SetColor)
	(
		rttr::metadata(META_COLOR, true)
	)
	.property("Is Rendering", &Image::IsRendering, &Image::SetIsRendering)
	.property("Has Premultiplied Alpha", &Image::HasPremultipliedAlpha, &Image::SetHasPremultipliedAlpha);
}