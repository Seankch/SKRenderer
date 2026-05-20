#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

/*****************************************************
    Includes
*****************************************************/
#include <Managers/ManagerBase.h>
#include <entt/entt.hpp>

// Required Manager
#include <Managers/ResourceManager.h>

class EntityManager : public ManagerBase
{
public:
	// Constructor/Destructor
	EntityManager(ResourceManager* _resMgr);
	~EntityManager();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// EntityManager functions
	entt::registry& GetECS(void);
private:
	// entt ECS
	entt::registry ecs;

	// Required Managers
    ResourceManager* resourceMgr;
};

#endif

