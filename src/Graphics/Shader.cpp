/*****************************************************
	Includes
*****************************************************/
#include <Graphics/Shader.h>
#include <iostream>
#include <Managers/RendererManager.h>

const std::vector<std::string> Shader::mBaseUniformVarList = { "UTime", "UWidth", "UHeight", "UTintColor", "lights", "dirLight", "UAmbientIntensity", "UAmbientColor", "UPointLightNum", "BloomPrePassTex" };

Shader::Shader() 
{
}

Shader::~Shader() 
{
}

void Shader::Start()
{
}

void Shader::End() 
{
}

void Shader::AddUniform(Uniform const& _uniform)
{
	mUniformVarList.push_back(_uniform);
}

std::vector<Uniform> const& Shader::GetUniformList(void) const
{
	return mUniformVarList;
}
