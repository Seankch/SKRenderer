#include <Components/Camera.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
	// Init camera variables
	mForward = { 0.f, 0.f, 1.f };
	mUp = { 0.f, 1.f, 0.f };
	mRight = { 1.f, 0.f, 0.f };
	mFov = 60.f;
	mNear = 0.1f;
	mFar = 10000.f;
	mAspectRatio = 16.f / 9.f;
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetViewXForm(void) const
{
	return mViewXform;
}

glm::mat4 Camera::GetProjXForm(void) const
{
	return mProjectionXform;
}

glm::vec3 Camera::GetPosition(void) const
{
	return mPosition;
}

void Camera::SetPosition(glm::vec3 _pos)
{
	mPosition = _pos;
	UpdateViewXForm();
}

glm::quat Camera::GetRotation()
{
	return mRotQuat;
}

void Camera::SetRotation(glm::vec3 const& _eulerAngles)
{
	mRotEulerAngles = _eulerAngles;

	// Convert euler angles to quaternion
	glm::quat x = glm::normalize(glm::angleAxis(glm::radians(mRotEulerAngles.x), glm::vec3(0.f, 1.f, 0.f)));
	glm::quat y = glm::normalize(glm::angleAxis(glm::radians(mRotEulerAngles.y), glm::vec3(1.f, 0.f, 0.f)));
	glm::quat z = glm::normalize(glm::angleAxis(glm::radians(mRotEulerAngles.z), glm::vec3(0.f, 0.f, 1.f)));
	mRotQuat = x * y * z;

	// Update view transform
	UpdateViewXForm();
}

void Camera::SetRotation(glm::quat const& _rotQuat)
{
	mRotQuat = _rotQuat;
	mRotEulerAngles = glm::eulerAngles(mRotQuat);
	UpdateViewXForm();
}

glm::vec3 Camera::GetForward(void) const
{
	return mForward;
}

glm::vec3 Camera::GetUp(void) const
{
	return mUp;
}

glm::vec3 Camera::GetRight(void) const
{
	return mRight;
}

float Camera::GetMoveSpeed(void) const
{
	return mMoveSpeed;
}

void Camera::SetMoveSpeed(float _speed)
{
	mMoveSpeed = _speed;
}

float Camera::GetRotSpeed(void) const
{
	return mRotSpeed;
}

void Camera::SetRotSpeed(float _speed)
{
	mRotSpeed = _speed;
}

float Camera::GetAspectRatio(void)
{
	return mAspectRatio;
}

void Camera::SetAspectRatio(float _aspectRatio)
{
	if (_aspectRatio > 0.f)
	{
		mAspectRatio = _aspectRatio;
		UpdateProjXForm();
	}
}

bool Camera::GetClamped(void)
{
	return mClamped;
}

void Camera::UpdateViewXForm(void)
{
	mForward = mRotQuat * glm::vec3{ 0.f, 0.f, -1.f };
	mRight = glm::normalize(glm::cross(mForward, glm::vec3{ 0.f, 1.f, 0.f }));
	mUp = glm::normalize(glm::cross(mRight, mForward));
	mTarget = mPosition + mForward;
	mViewXform = glm::lookAt(mPosition, mTarget, mUp);
}

void Camera::UpdateProjXForm(void)
{
	mProjectionXform = glm::perspective(glm::radians(mFov), mAspectRatio, mNear, mFar);
}

void Camera::MoveCamera(glm::vec3 const& _moveDir)
{
	mPosition += _moveDir * mMoveSpeed;
	SetPosition(mPosition);
}

void Camera::RotateCamera(glm::vec3 const& _rotOffset)
{
	// nand check
	if (glm::all(glm::isnan(_rotOffset)))
	{
		return;
	}

	// Rotate camera
	mRotEulerAngles += _rotOffset * mRotSpeed;

	if (mClamped)
	{
		mRotEulerAngles.y = glm::clamp(mRotEulerAngles.y, -85.0f, 85.0f);
	}

	SetRotation(mRotEulerAngles);
}

void Camera::SetClamped(bool _clamped)
{
	mClamped = _clamped;
}

bool Camera::GetIsMainCam(void)
{
	return mIsMainCam;
}

void Camera::SetIsMainCam(bool _isMainCam)
{
	mIsMainCam = _isMainCam;
}

void Camera::Reset(void)
{
	mPosition = { 0.f, 0.f, 10.f };
	mForward = { 0.f, 0.f, 1.f };
	mUp = { 0.f, 1.f, 0.f };
	mRight = { 1.f, 0.f, 0.f };
	mFov = 60.f;
	mNear = 0.1f;
	mFar = 10000.f;
	mAspectRatio = 16.f / 9.f;

	SetRotation(glm::vec3{ 0.f, 0.f, 0.f });
	UpdateProjXForm();
	UpdateViewXForm();
}

//RTTR_REGISTRATION
//{
//		rttr::registration::class_<Camera>("Camera")
//		.property("Position", &Camera::GetPosition, &Camera::SetPosition)
//		.property("Move Speed", &Camera::GetMoveSpeed, &Camera::SetMoveSpeed)
//		.property("Rotation Speed", &Camera::GetRotSpeed, &Camera::SetRotSpeed)
//		.property("Aspect Ratio", &Camera::GetAspectRatio, &Camera::SetAspectRatio)
//		.property("Is camera clamped?", &Camera::GetClamped, &Camera::SetClamped)
//
//		// read-only properties
//		.property_readonly("View Transform", &Camera::GetViewXForm)
//		.property_readonly("Projection Transform", &Camera::GetProjXForm)
//		.property_readonly("Forward", &Camera::GetForward)
//		.property_readonly("Up", &Camera::GetUp)
//		.property_readonly("Right", &Camera::GetRight)
//	
//		// overloaded functions
//		.method("SetRotation", rttr::select_overload<void(glm::vec3 const&)>(&Camera::SetRotation))
//		.method("SetRotation", rttr::select_overload<void(glm::quat const&)>(&Camera::SetRotation))
//	
//		// custom functions
//		.method("MoveCamera", &Camera::MoveCamera)
//		.method("RotateCamera", &Camera::RotateCamera);
//}