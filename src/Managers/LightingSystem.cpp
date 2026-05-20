/*****************************************************
    Includes
*****************************************************/
#include <Managers/LightingSystem.h>
#include <Managers/GraphicsSystem.h>

LightingSystem::LightingSystem(EntityManager* _entityMgr, RendererManager* _rendererMgr) :
	ambientLightIntensity{}, 
	ambientLightColor{}, 
	entityMgr{ _entityMgr },
    rendererMgr{ _rendererMgr }
{
}

void LightingSystem::Load()
{
}

void LightingSystem::LateLoad()
{
}

void LightingSystem::Init()
{
	// Init ambient light
	ambientLightColor = { 1.f, 1.f, 1.f };
	ambientLightIntensity = 1.f;
}

void LightingSystem::Update()
{
	// Get main renderer
	RendererBase* mainRenderer = rendererMgr->GetRenderer();

	// Pass ambient light to shader
	mainRenderer->SetAmbientLight(ambientLightColor, ambientLightIntensity);

	// Iterate and set all light components in scene
	auto& ecs = entityMgr->GetECS();
	int pointLightCount{}, dirLightCount{};
	for (auto& ent : ecs.view<Light>())
	{
		// Check if entity has transform & light component
		if (!ecs.all_of<Transform, Light>(ent))
			continue;

		// Entity has transform & light component, begin render
		Transform& xform = ecs.get<Transform>(ent);
		Light& light = ecs.get<Light>(ent);

		// If light type is directional, pass properties to shader
		if (light.GetLightType() == Light::LT_DIRECTIONAL)
		{
			// Pass uniforms to shader
			mainRenderer->SetDirectionalLight(light.GetDirection(), light.GetLightColor() * light.GetIntensity(), dirLightCount);
			++dirLightCount;
		}
		else if (light.GetLightType() == Light::LT_POINT)
		{
			// Pass uniforms to shader
			mainRenderer->SetPointLight(xform.GetPos(), light.GetLightColor() * light.GetIntensity(), light.GetRange(), pointLightCount);
			++pointLightCount;
		}
	}

	// Pass light count to shader
	mainRenderer->SetLightCounts(pointLightCount, dirLightCount);
}

void LightingSystem::FixedUpdate()
{
}

void LightingSystem::LateUpdate()
{
}

void LightingSystem::Exit()
{
}

void LightingSystem::Unload()
{
}

float LightingSystem::GetAmbientIntensity(void)
{
	return ambientLightIntensity;
}

glm::vec3 LightingSystem::GetAmbientColor(void)
{
	return ambientLightColor;
}

void LightingSystem::SetAmbientIntensity(float _intensity)
{
	ambientLightIntensity = _intensity;
}

void LightingSystem::SetAmbientColor(glm::vec3 const& _color)
{
	ambientLightColor = _color;
}
