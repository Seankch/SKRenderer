#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include <Graphics/Material.h>
#include <Managers/EntityManager.h>
#include <Managers/ResourceManager.h>

class MaterialManager : public ManagerBase
{
public:
	// Constructor/desctructor
	MaterialManager(ResourceManager* _resourceMgr, EntityManager* _entityMgr, JSONManager* _jsonMgr);
	~MaterialManager();

	// Manager functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// Saving and loading of materials
	void CreateNewMaterial(std::string const& _matName);
	void SaveMaterial(Material& _mat);

	// Assigning material instances
	void AssignAllMaterialInstance(void);
	void ClearMaterialInstances(void);
private:
	// Required managers
	ResourceManager* resourceMgr;
	EntityManager* entityMgr;
	JSONManager* jsonManager;
};

#endif
