#ifndef RAYTRACEMANAGER_H
#define RAYTRACEMANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include <Graphics/GraphicsDefine.h>
#include <Managers/ManagerBase.h>

// Required managers
#include <Managers/ResourceManager.h>
#include <Managers/RendererManager.h>
#include <Managers/EntityManager.h>

class RaytraceManager : public ManagerBase
{
public:
    RaytraceManager(EntityManager* _entityMgr, ResourceManager* _resourceMgr, RendererManager* _rendererMgr);
    ~RaytraceManager();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

    // Raytrace manager functions
    void CreateAccelerationStructures(void);
    void CreateLUTResources(void);
	void CreateReSTIRResources(void);
private:
	// Required managers
	EntityManager* entityMgr{};
    ResourceManager* resourceMgr{};
    RendererManager* rendererMgr{};
};

#endif