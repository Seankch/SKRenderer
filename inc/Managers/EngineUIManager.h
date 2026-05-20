#ifndef ENGINE_UI_MANAGER_H
#define ENGINE_UI_MANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include <Managers/ManagerBase.h>

// Required managers
#include <Managers/EntityManager.h>
#include <Managers/RendererManager.h>
#include <Managers/LightingSystem.h>

class EngineUIManager : public ManagerBase
{
public:
	// Constructor/Destructor
	EngineUIManager(EntityManager* _em, RendererManager* _rm, LightingSystem* _ls);
	~EngineUIManager();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// EngineUI functions
	void Render();
	void PostRender();
private:
	// GUI windows
	void MainPanel(void);
	void GraphicsPanel(void);

	// Required Managers
	EntityManager* entityMgr = nullptr;
	RendererManager* rendererMgr = nullptr;
    LightingSystem* lightingSystem = nullptr;

	// Main panel variables
	entt::entity mSelected = entt::null;
	int mSelectedIndex{};

	// Material display
	std::unordered_map<entt::entity, int> mSelectedMatInstance;
};

#endif