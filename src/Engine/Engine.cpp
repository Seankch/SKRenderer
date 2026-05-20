#include "Engine/Engine.h"

// Required managers
#include <Managers/WindowsManager.h>
#include <Managers/ResourceManager.h>
#include <Managers/EntityManager.h>
#include <Managers/RendererManager.h>
#include <Managers/GraphicsSystem.h>
#include <Managers/EngineUIManager.h>
#include <Managers/JSONManager.h>
#include <Managers/MaterialManager.h>
#include <Managers/RaytraceManager.h>
#include <Managers/AssetManager.h>
#include <Managers/LightingSystem.h>

void Engine::Init()
{
    // Manually create and register all managers
    diContainer.Add(new WindowsManager());
    diContainer.Add(new JSONManager());
    diContainer.Add(new AssetManager());
    diContainer.Add(new RendererManager(diContainer.Get<WindowsManager>()));
    diContainer.Add(new ResourceManager(diContainer.Get<RendererManager>(), diContainer.Get<JSONManager>()));
    diContainer.Add(new EntityManager(diContainer.Get<ResourceManager>()));
    diContainer.Add(new MaterialManager(diContainer.Get<ResourceManager>(), diContainer.Get<EntityManager>(), diContainer.Get<JSONManager>()));
    diContainer.Add(new LightingSystem(diContainer.Get<EntityManager>(), diContainer.Get<RendererManager>()));
    diContainer.Add(new EngineUIManager(diContainer.Get<EntityManager>(), diContainer.Get<RendererManager>(), diContainer.Get<LightingSystem>()));
    diContainer.Add(new RaytraceManager(diContainer.Get<EntityManager>(), diContainer.Get<ResourceManager>(), diContainer.Get<RendererManager>()));
    diContainer.Add(new GraphicsSystem(diContainer.Get<WindowsManager>(), diContainer.Get<ResourceManager>(), diContainer.Get<EntityManager>(), diContainer.Get<RendererManager>()));

    // Load all managers
    std::vector<ManagerBase*>& containerList = diContainer.GetContainerList();
    for (int i = 0; i < containerList.size(); ++i)
    {
        containerList[i]->Load();
    }

    // Load additional stuff after all managers have loaded
    for (int i = 0; i < containerList.size(); ++i)
    {
        containerList[i]->LateLoad();
    }
}

void Engine::Update()
{
    // Get container
    std::vector<ManagerBase*>& containerList = diContainer.GetContainerList();
    
    // Init all managers
    for (int i = 0; i < containerList.size(); ++i) 
    {
        containerList[i]->Init();
    }

    // Get graphics system and engine UI manager
    GraphicsSystem* graphicsSystem = diContainer.Get<GraphicsSystem>();
    EngineUIManager* engineUIMgr = diContainer.Get<EngineUIManager>();

    // Main game loop
    GLFWwindow* windowPtr = diContainer.Get<WindowsManager>()->mPtrWindow;
    while (!glfwWindowShouldClose(windowPtr))
    {
        // Update managers
        for (int i = 0; i < containerList.size(); ++i) 
        {
            containerList[i]->Update();
            containerList[i]->FixedUpdate();
        }

        // Late Update
        for (int i = 0; i < containerList.size(); ++i) 
        {
            containerList[i]->LateUpdate();
        }

        // Render entities and UI
        engineUIMgr->Render();
        graphicsSystem->Render();
        engineUIMgr->PostRender();

        // Swap buffers
        glfwSwapBuffers(windowPtr);
    }

    // Once game loop is terminated, exit managers
    for (int i = 0; i < containerList.size(); ++i) 
    {
        containerList[i]->Exit();
    }
}

void Engine::Exit()
{
    // Get container
    std::vector<ManagerBase*>& containerList = diContainer.GetContainerList();

    // Unload all managers from bottom up
    int startIdx = static_cast<int>(containerList.size()) - 1;
    for (int i = startIdx; i >= 0; --i)
    {
        containerList[i]->Unload();
        delete containerList[i];
    }
}
