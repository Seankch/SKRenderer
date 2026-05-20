#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

/*****************************************************
    Includes
*****************************************************/
#include <Components/Light.h>
#include <Graphics/Shader.h>

// Required Managers
#include <Managers/EntityManager.h>
#include <Managers/RendererManager.h>

class LightingSystem : public ManagerBase
{
public:
	LightingSystem(EntityManager* _entityMgr, RendererManager* _rendererMgr);

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// For getting/setting ambient light
	float GetAmbientIntensity(void);
	glm::vec3 GetAmbientColor(void);
	void SetAmbientIntensity(float _intensity);
	void SetAmbientColor(glm::vec3 const& _color);
private:
	// Ambient light
	float ambientLightIntensity;
	glm::vec3 ambientLightColor;

	// Required Managers
	EntityManager* entityMgr{};
    RendererManager* rendererMgr{};
};

#endif
