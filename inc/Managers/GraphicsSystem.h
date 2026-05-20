#ifndef GRAPHICS_SYSTEM_H
#define GRAPHICS_SYSTEM_H

/*****************************************************
    Includes
*****************************************************/
#include <Managers/ManagerBase.h>
#include <Managers/WindowsManager.h>
#include <Managers/ResourceManager.h>
#include <Managers/EntityManager.h>
#include <Managers/RendererManager.h>
#include "Components/Camera.h"
#include <Components/MeshRenderer.h>
#include <Components/Transform.h>
#include <Components/Text.h>

#pragma warning(push, 0)
#include <document.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#pragma warning(pop)

#define SCREEN_STR "Screen"

class GraphicsSystem : public ManagerBase
{
public:
	enum VIEWPORT_WINDOW
	{
		VW_GAME = 0,
		VW_EDITOR = 1
	};

    enum RENDERER_TYPE
    {
        RT_RASTERIZE = 0,
        RT_RAYTRACE = 1
    };

	// Constructor
	GraphicsSystem(WindowsManager* _wm, ResourceManager* _rm, EntityManager* _em, RendererManager* _rendererMgr);
	~GraphicsSystem();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// Graphics system functions
	void Render();
	void RenderRasterize();
    void RenderRayTrace();

	// Other functions
	Camera* GetCamera(VIEWPORT_WINDOW _viewport);
	void SetCamera(VIEWPORT_WINDOW _viewport, Camera* _cam); // Called when camera is de-serialized to set editor/game camera.
	
	// For maximizing game view
	void SetIsMaximized(bool _isMaximized);
	bool GetIsMaximized(void);
private:
	// Graphics system variables
	std::unordered_map<VIEWPORT_WINDOW, Camera*> mCameras;
	
	// For maximizing game view
	bool mIsMaximized;

    // For renderer type
    RENDERER_TYPE mRendererType;

	// Required managers
	WindowsManager* windowsMgr = nullptr;
	ResourceManager* resourceMgr = nullptr;
	EntityManager* entityMgr = nullptr;
	RendererManager* rendererMgr = nullptr;
};

#endif