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

glm::vec3 Transform::GetForwardVector() const {
    return glm::normalize(mRot * glm::vec3(0.f, 0.f, -1.f));
}

glm::vec3 Transform::GetRightVector() const {
    return glm::normalize(mRot * glm::vec3(1.f, 0.f, 0.f));
}

glm::vec3 Transform::GetUpVector() const {
    return glm::normalize(mRot * glm::vec3(0.f, 1.f, 0.f));
}

//RTTR_REGISTRATION
//{
//  rttr::registration::class_<Transform>("Transform")
//  .property("Position", &Transform::GetPos, &Transform::SetPos)
//  .property("Rotation", &Transform::GetRot, &Transform::SetRot)
//  .property("Scale", &Transform::GetScale, &Transform::SetScale);
//}
