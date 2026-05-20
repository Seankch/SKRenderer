#ifndef UI_MANAGER_H
#define UI_MANAGER_H

/*****************************************************
	Includes
*****************************************************/
#include <Components/Image.h>
#include <Graphics/Shader.h>

class UIManager
{
public:
	//Manager Functions
	void Init();
	void Load();
	void Update();
	void FixedUpdate();
	void Free();
	void Unload();

	// UI Manager functions
	void Render(glm::mat4 const& _projXForm);

	//Lerp
	glm::vec3 LerpButton(float _timer, float _duration, glm::vec3 _start, glm::vec3 _end);

	//Used for switching of scenes to hide next scene's menu UI
	bool mHideAllMenus = false;

	//UI Index
	int mCurrUiIndex = -1;

	//Play sound once
	bool mSoundPlayed = false;

	//Toggling of menus
	void SetMenu(std::string _targetMenu, bool _openMenu);
	void PlayMenuSoundOnce(std::string _targetSoundFile);
private:
	//For singleton
	UIManager();
	friend Singleton<UIManager>;

	bool mIsRenderUI;
};

#endif