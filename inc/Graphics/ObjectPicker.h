#ifndef OBJECT_PICKER_H
#define OBJECT_PICKER_H

/*****************************************************
	Includes
*****************************************************/
#include <Graphics/Texture.h>
#include <Graphics/GraphicsDefine.h>
#include <Managers/GraphicsSystem.h>
#include <Components/Transform.h>

class ObjectPicker
{
public:
	// Constructor
	ObjectPicker();

	// Object picker functions
	void Init(GraphicsSystem::VIEWPORT_WINDOW _window, std::string const& _viewportName);

	// Check for object hover (FBO based)
	void WriteToFBO(GraphicsSystem::VIEWPORT_WINDOW _window);
	void WriteToFBOImage(GraphicsSystem::VIEWPORT_WINDOW _window);
	uint32_t ReadPixel(GraphicsSystem::VIEWPORT_WINDOW _window); // Returns ID of entity
	void FreeObjectPickerFBO(GraphicsSystem::VIEWPORT_WINDOW _window);
	bool IsUIHovered(entt::entity _id); // Only used for game viewport

	// Check for object hover (Ray based)
	bool IsMouseHoverEditor(Transform const& _transform);
private:
	// Object picker variables
	std::unordered_map<GraphicsSystem::VIEWPORT_WINDOW, std::string> mViewportNameMap;
	std::unordered_map<std::string, std::string> mFBONameMap;

	// Helper function for object picker (Ray based)
	bool RaySphereIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter, float sphereRadius);
};

#endif