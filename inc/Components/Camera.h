#ifndef CAMERA_H
#define CAMERA_H

#include <Graphics/GraphicsDefine.h>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
    Camera();
    ~Camera();

    // Getters/Setters
    glm::mat4 GetViewXForm(void) const;
    glm::mat4 GetProjXForm(void) const;
    glm::vec3 GetPosition(void) const;
    void SetPosition(glm::vec3 _pos);
    glm::quat GetRotation();
    void SetRotation(glm::vec3 const& _eulerAngles);
    void SetRotation(glm::quat const& _rotQuat);
    glm::vec3 GetForward(void) const;
    glm::vec3 GetUp(void) const;
    glm::vec3 GetRight(void) const;
    float GetMoveSpeed(void) const;
    void SetMoveSpeed(float _speed);
    float GetRotSpeed(void) const;
    void SetRotSpeed(float _speed);
    float GetAspectRatio(void);
    void SetAspectRatio(float _aspectRatio);
    bool GetClamped(void);
    void SetClamped(bool _clamped);
    bool GetIsMainCam(void);
    void SetIsMainCam(bool _isMainCam);

    // Update transform matrices
    void UpdateViewXForm(void);
    void UpdateProjXForm(void);

    // Functions for updating camera
    void MoveCamera(glm::vec3 const& _moveDir);
    void RotateCamera(glm::vec3 const& _rotOffset);
	void Reset(void);
private:
    // Transform matrices
    glm::mat4 mViewXform{};
    glm::mat4 mProjectionXform{};

    // Camera logic variables
    glm::vec3 mPosition{}, mTarget{};
    glm::vec3 mRotEulerAngles{};
    glm::quat mRotQuat{};
    float mMoveSpeed{}, mRotSpeed{};

    // Camera variables
    glm::vec3 mForward, mUp, mRight;
    float mFov;
    float mNear, mFar;
    float mAspectRatio;
    bool mIsMainCam = false;

    // Misc variables
    bool mClamped = false;
};

#endif