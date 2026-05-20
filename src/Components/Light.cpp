/*****************************************************
		Includes
*****************************************************/
#include <Components/Light.h>

Light::Light() : mLightType{ LT_POINT }, mIntensity{}, mColor{}, mDirection{}, mRange{}
{
}

Light::~Light()
{
}

glm::vec3 Light::GetLightColor(void) const
{
	return mColor;
}

void Light::SetLightColor(glm::vec3 _color)
{
	mColor = _color;
}

Light::LIGHT_TYPE Light::GetLightType(void) const
{
	return mLightType;
}

void Light::SetLightType(LIGHT_TYPE _type)
{
	mLightType = _type;
}

float Light::GetIntensity(void) const
{
	return mIntensity;
}

void Light::SetIntensity(float _intensity)
{
	mIntensity = _intensity;
}

glm::vec3 Light::GetDirection(void) const
{
	return mDirection;
}

void Light::SetDirection(glm::vec3 _dir)
{
	mDirection = _dir;
}

float Light::GetRange(void) const
{
	return mRange;
}

void Light::SetRange(float _range)
{
    mRange = _range;
}
