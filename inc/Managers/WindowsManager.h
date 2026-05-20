#ifndef WINDOWS_MANAGER_H
#define WINDOWS_MANAGER_H

#include <GLFW/glfw3.h>
#include "Managers/ManagerBase.h"
#include <vector>
#include <string>

class WindowsManager : public ManagerBase
{
private:
	enum WINDOW_MODE
	{
		WM_FULLSCREEN = 0,
		//WM_BORDERLESS,
		WM_WINDOWED,
		WM_COUNT
	};

public:
	// Constructor/Destructor
	WindowsManager();
	~WindowsManager();

	// Manager Functions
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// Window creation
	bool CreateWindowLocal(int _width, int _height, std::string _winTitle, double _fpsUpdateInterval = 1.f);

	// Updating Delta time
	void UpdateTime();

	// Callback functions
	static void ErrorCallback(int _error, const char* _description);
	static void ResizeFBOCallback(GLFWwindow* _winPtr, int _width, int _height);
	static void ResizeWindowCallback(GLFWwindow* _winPtr, int _width, int _height);
	static void FocusCallback(GLFWwindow* _window, int _focused);

	// Switching of window modes
	void SwitchWindowMode(WINDOW_MODE _nextMode);

	// Window variables
	GLFWwindow* mPtrWindow;
	static int mWidth;          // Window width
	static int mHeight;         // Window height (including the title bar)
	static int mDrawableWidth;  // Window width (drawable area only)
	static int mDrawableHeight; // Window height (drawable area only, excluding title bar)
	std::string mTitle;

	// Time related
	double mDeltaTime;
	double mDTThreshold;
	double mElapsedTime;

	// FPS
	double mFPS;

	// Check if graphics needs to resize
	static bool mResizeGraphics;
private:
	// Time calculation
	double mPrevTime;

	// FPS calculation
	unsigned mGameLoopCount;
	double mStartTime;
	double mFPSUpdateInterval;

	WINDOW_MODE mWindowMode{ WM_FULLSCREEN }; // Defaulted to fullscreen mode
	bool mWindowSwapped{ false };
};

#endif
