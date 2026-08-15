#include "Components/Transform.h"

glm::vec3 Transform::GetPos(void) const
{
    return mPos;
}

glm::vec3 Transform::GetScale(void) const
{
    return mScale;
}

glm::vec3 Transform::GetRot(void) const
{
    return glm::degrees(glm::eulerAngles(mRot));
}

void Transform::SetPos(glm::vec3 _pos)
{
    // Update position
    mPos = _pos;
}

void Transform::SetScale(glm::vec3 _scale)
{
    // Update scale
    mScale = _scale;
}

void Transform::SetRot(glm::vec3 _rot)
{
    glm::vec3 rotRadians = glm::radians(_rot);
    mRot = glm::quat(rotRadians);
}
