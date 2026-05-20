/*****************************************************
	Includes
*****************************************************/
#include <windows.h>
#include "Managers/WindowsManager.h"
#include <Managers/GraphicsSystem.h>
#include <iostream>

#define UNREFERENCED_PARAMETER(P) (P)

int WindowsManager::mWidth = 0;
int WindowsManager::mHeight = 0;
int WindowsManager::mDrawableWidth = 0;
int WindowsManager::mDrawableHeight = 0;
bool WindowsManager::mResizeGraphics = false;

WindowsManager::WindowsManager()
	: mPtrWindow(nullptr)
	, mTitle("")
	, mDeltaTime(0)
	, mFPS(0)
	, mElapsedTime(0)
	, mPrevTime(0)
	, mGameLoopCount(0)
	, mStartTime(0)
	, mFPSUpdateInterval(0)
	, mDTThreshold(0.1)
{
}

WindowsManager::~WindowsManager()
{
}

void WindowsManager::Load()
{
	// Init GLFW
	if (!glfwInit())
		return;

	// Init window without graphics api first
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Init variables
	mGameLoopCount = 0;
	mPrevTime = mStartTime = glfwGetTime();

	// Create window
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	std::string windowTitle = "SKRenderer";
	if ((mode->width / 16 * 9) <= mode->height)
	{
		CreateWindowLocal(mode->width, mode->width / 16 * 9, windowTitle, 1.0);
	}
	else
	{
		CreateWindowLocal(mode->height / 9 * 16, mode->height, windowTitle, 1.0);
	}

	// Set callback functions
	glfwSetErrorCallback(ErrorCallback);
	glfwSetWindowSizeCallback(mPtrWindow, ResizeWindowCallback);
	glfwSetWindowFocusCallback(mPtrWindow, FocusCallback);
	glfwSetFramebufferSizeCallback(mPtrWindow, ResizeFBOCallback);

	// Set drawable area
	glfwGetFramebufferSize(mPtrWindow, &mDrawableWidth, &mDrawableHeight);

	// Make context current and enable vsync
	glfwMakeContextCurrent(mPtrWindow);
	glfwSwapInterval(1);
}

void WindowsManager::LateLoad()
{
}

void WindowsManager::Init()
{
}

void WindowsManager::Update()
{
	// Update time on all windows
	UpdateTime();

	// Poll events
	glfwPollEvents();
}

void WindowsManager::FixedUpdate()
{
}

void WindowsManager::LateUpdate()
{
}

void WindowsManager::Exit()
{
}

void WindowsManager::Unload()
{
	// Destroy window
	glfwDestroyWindow(mPtrWindow);
	glfwTerminate();
}

bool WindowsManager::CreateWindowLocal(int _width, int _height, std::string _winTitle, double _fpsUpdateInterval)
{
	//Set variables
	mWidth = _width;
	mHeight = _height;
	mTitle = _winTitle;
	mFPSUpdateInterval = _fpsUpdateInterval;

	// Create window
	mPtrWindow = glfwCreateWindow(_width, _height, _winTitle.c_str(), NULL, NULL);
	// mPtrWindow = glfwCreateWindow(_width, _height, _winTitle.c_str(), glfwGetPrimaryMonitor(), NULL); // For fullscreen
	if (!mPtrWindow)
	{
		// Window creation failed
		glfwTerminate();
		return false;
	}

	// Window created successfully, return true
	return true;
}

void WindowsManager::UpdateTime()
{
	//Elapsed time (Time between prev frame and current frame)
	double currTime = glfwGetTime();
	mDeltaTime = currTime - mPrevTime;
	if (mDeltaTime > mDTThreshold)
		mDeltaTime = mDTThreshold;

	mPrevTime = currTime;

	//FPS
	mGameLoopCount++;
	double elapsedTime = currTime - mStartTime;

	//Update FPS based on update interval
	if (elapsedTime >= mFPSUpdateInterval)
	{
		mFPS = mGameLoopCount / elapsedTime;
		mStartTime = currTime;
		mGameLoopCount = 0;
	}

	mElapsedTime += mDeltaTime;
}

// Callback functions
void WindowsManager::ErrorCallback(int _error, const char* _description)
{
	UNREFERENCED_PARAMETER(_error);

#ifdef _ENGINE
	DebugLogger::GetInstance()->Log("GLFW Error: %s\n", _description);
#else
	UNREFERENCED_PARAMETER(_description);
#endif

}

void WindowsManager::ResizeFBOCallback(GLFWwindow* _winPtr, int _width, int _height)
{
	// Set window width/height
	mWidth = _width;
	mHeight = _height;

	// Set drawable area size (without title bar, etc)
	glfwGetFramebufferSize(_winPtr, &mDrawableWidth, &mDrawableHeight);

	// Tell renderer to resize
	mResizeGraphics = true;
}

void WindowsManager::ResizeWindowCallback(GLFWwindow* _winPtr, int _width, int _height)
{
	// Set window size
	glfwSetWindowSize(_winPtr, _width, _height);

	// Handle minimize
	glfwGetFramebufferSize(_winPtr, &_width, &_height);
	while ((_width == 0) || (_height == 0)) 
	{
		glfwGetFramebufferSize(_winPtr, &_width, &_height);
		glfwWaitEvents();
	}

	// Set window size
	glfwSetWindowSize(_winPtr, _width, _height);
}

void WindowsManager::FocusCallback(GLFWwindow* _window, int _focused)
{
	UNREFERENCED_PARAMETER(_window);
	UNREFERENCED_PARAMETER(_focused);
}

void WindowsManager::SwitchWindowMode(WINDOW_MODE _nextMode)
{
	switch (_nextMode)
	{
	case WM_FULLSCREEN:
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowAttrib(mPtrWindow, GLFW_RESIZABLE, GLFW_TRUE);
			glfwSetWindowAttrib(mPtrWindow, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(mPtrWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		break;
	//case WM_BORDERLESS:
	//	{
	//		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	//		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	//		glfwMaximizeWindow(mPtrWindow);
	//		glfwSetWindowAttrib(mPtrWindow, GLFW_RESIZABLE, GLFW_FALSE);
	//		glfwSetWindowAttrib(mPtrWindow, GLFW_DECORATED, GLFW_FALSE);
	//		glfwSetWindowMonitor(mPtrWindow, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
	//	}
	//	break;
	case WM_WINDOWED:
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowAttrib(mPtrWindow, GLFW_RESIZABLE, GLFW_TRUE);
			glfwSetWindowAttrib(mPtrWindow, GLFW_DECORATED, GLFW_TRUE);

			//Center window on monitor
			int windowWidth = 1920;
			int windowHeight = 1080;
			int xPos = (int)(mode->width / 2.f - windowWidth / 2.f);
			int yPos = (int)(mode->height / 2.f - windowHeight / 2.f);

			glfwSetWindowMonitor(mPtrWindow, NULL, xPos, yPos, windowWidth, windowHeight, mode->refreshRate);
		}
		break;
	}
}
