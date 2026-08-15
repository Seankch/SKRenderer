#ifndef TRANSFORM_H
#define TRANSFORM_H

/*****************************************************
		Includes
*****************************************************/
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::translate, glm::scale
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>       // For glm::toMat4
#include <glm/gtx/matrix_decompose.hpp> // For glm::decompose
#include <entt/entt.hpp>

class Transform
{
public:
	Transform() = default;

	// Getters/Setters
	glm::vec3 GetPos(void) const;
	glm::vec3 GetScale(void) const;
	glm::vec3 GetRot(void) const;
	void SetPos(glm::vec3 _pos);
	void SetScale(glm::vec3 _scale);
	void SetRot(glm::vec3 _rot);
private:
	glm::vec3 mPos{};
	glm::vec3 mScale{};
	glm::quat mRot{};
};

#endif
