#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include <Managers/ManagerBase.h>
#include <entt/entt.hpp>

class AssetManager : public ManagerBase
{
public:
	// Constructor/Destructor
	AssetManager();
	~AssetManager();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

private:
	void CompileMeshes(std::string const& _filePath);
	void CompileTextures(std::string const& _filePath);
	void CompileShaders(std::string const& _filePath);
	void RunExecutable(std::string const& _executablePath);
};

#endif