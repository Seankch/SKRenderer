#include <Managers/EntityManager.h>
#include <Components/Transform.h>
#include <Components/Camera.h>
#include <Components/MeshRenderer.h>
#include <Components/Light.h>

EntityManager::EntityManager(ResourceManager* _resMgr) 
: resourceMgr(_resMgr)
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::Load()
{
	// Create camera object
	entt::entity tempEnt = ecs.create();
	// Assign camera component
	Camera camComp{};
	camComp.SetIsMainCam(true);
	camComp.UpdateViewXForm();
	camComp.UpdateProjXForm();
	ecs.emplace<Camera>(tempEnt, camComp);

	// Create sponza object
	tempEnt = ecs.create();
	// Assign transform component
	Transform sponzaTransformComp{};
	sponzaTransformComp.SetPos(glm::vec3{ 0.f, -22.5f, -30.f });
	sponzaTransformComp.SetScale(glm::vec3{ 0.2f, 0.2f, 0.2f });
	sponzaTransformComp.SetRot(glm::vec3{-90.f, -90.f, 0.f});
	ecs.emplace<Transform>(tempEnt, sponzaTransformComp);
	// Assign MeshRenderer component
	MeshRenderer sponzaMeshComp{};
	sponzaMeshComp.SetModel(*resourceMgr->GetResource<Model>("sponza"));

	// Set materials, hardcoded for now
	// Will be loaded from scene file in the future
	sponzaMeshComp.SetMaterial(0, "curtain_fabric_blue");
	sponzaMeshComp.SetMaterial(1, "curtain_fabric_red");
	sponzaMeshComp.SetMaterial(2, "curtain_fabric_green");
	sponzaMeshComp.SetMaterial(3, "arch_stone_wall_01");
	sponzaMeshComp.SetMaterial(4, "brickwall_01");
	sponzaMeshComp.SetMaterial(5, "brickwall_02");
	sponzaMeshComp.SetMaterial(6, "ceiling_plaster_01");
	sponzaMeshComp.SetMaterial(7, "ceiling_plaster_02");
	sponzaMeshComp.SetMaterial(8, "col_1stfloor");
	sponzaMeshComp.SetMaterial(9, "col_brickwall_01");
	sponzaMeshComp.SetMaterial(10, "col_head_1stfloor");
	sponzaMeshComp.SetMaterial(11, "col_head_2ndfloor_02");
	sponzaMeshComp.SetMaterial(12, "curtain_fabric_blue");
	sponzaMeshComp.SetMaterial(13, "curtain_fabric_green");
	sponzaMeshComp.SetMaterial(14, "curtain_fabric_red");
	sponzaMeshComp.SetMaterial(15, "curtain_fabric_blue");
	sponzaMeshComp.SetMaterial(16, "curtain_fabric_green");
	sponzaMeshComp.SetMaterial(17, "curtain_fabric_red");
	sponzaMeshComp.SetMaterial(18, "curtain_fabric_blue");
	sponzaMeshComp.SetMaterial(19, "curtain_fabric_green");
	sponzaMeshComp.SetMaterial(20, "curtain_fabric_red");
	sponzaMeshComp.SetMaterial(21, "door_stoneframe_01");
	sponzaMeshComp.SetMaterial(22, "door_stoneframe_02");
	sponzaMeshComp.SetMaterial(23, "floor_tiles_01");
	sponzaMeshComp.SetMaterial(24, "Glass"); // glass
	sponzaMeshComp.SetMaterial(25, "Glass"); // lamp_glass
	sponzaMeshComp.SetMaterial(26, "Lightbulb"); // light_bulb
	sponzaMeshComp.SetMaterial(27, "metal_door_01");
	sponzaMeshComp.SetMaterial(28, "ornament_01");
	sponzaMeshComp.SetMaterial(29, "lionhead_01");
	sponzaMeshComp.SetMaterial(30, "roof_tiles_01");
	sponzaMeshComp.SetMaterial(31, "stone_trims_01");
	sponzaMeshComp.SetMaterial(32, "stone_trims_02");
	sponzaMeshComp.SetMaterial(33, "stone_01_tile");
	sponzaMeshComp.SetMaterial(34, "stones_2ndfloor_01");
	sponzaMeshComp.SetMaterial(35, "window_frame_01");
	sponzaMeshComp.SetMaterial(36, "wood_tile_01");
	sponzaMeshComp.SetMaterial(37, "wood_door_01");
	ecs.emplace<MeshRenderer>(tempEnt, sponzaMeshComp);

	// Create ogre object
	tempEnt = ecs.create();
	// Assign transform component
	Transform planeTransformComp{};
	planeTransformComp.SetPos(glm::vec3{ 0.f, -8.f, -80.f });
	planeTransformComp.SetRot(glm::vec3{ 0.f, 0.f, 0.f });
	planeTransformComp.SetScale(glm::vec3{ 20.f, 20.f, 20.f });
	ecs.emplace<Transform>(tempEnt, planeTransformComp);
	// Assign MeshRenderer component
	MeshRenderer planeMeshComp{};
	planeMeshComp.SetModel(*resourceMgr->GetResource<Model>("ogre"));
	planeMeshComp.SetMaterial(0, "ogre");
	ecs.emplace<MeshRenderer>(tempEnt, planeMeshComp);

	// Create dir light object
	tempEnt = ecs.create();
	// Assign transform component
	Transform lightTransformComp{};
	lightTransformComp.SetPos(glm::vec3{ 0.f, 25.f, -80.f });
	ecs.emplace<Transform>(tempEnt, lightTransformComp);
	// Assign light component
	Light lightComp{};
	lightComp.SetLightType(Light::LT_DIRECTIONAL);
	lightComp.SetDirection(glm::vec3(0.f, -0.5f, -0.1f));
	lightComp.SetIntensity(3.f);
	lightComp.SetLightColor(glm::vec3(1.f, 1.f, 0.75f));
	ecs.emplace<Light>(tempEnt, lightComp);

	// Create point light object
	tempEnt = ecs.create();
	// Assign transform component
	Transform ptLightTransformComp{};
	ptLightTransformComp.SetPos(glm::vec3{ 0.f, 25.f, -128.f });
	ecs.emplace<Transform>(tempEnt, ptLightTransformComp);
	// Assign light component
	Light ptLightComp{};
	ptLightComp.SetLightType(Light::LT_POINT);
	ptLightComp.SetRange(125.f);
	ptLightComp.SetIntensity(2.5f);
	ptLightComp.SetLightColor(glm::vec3(1.f, 1.f, 1.f));
	ecs.emplace<Light>(tempEnt, ptLightComp);
	
	// Assign mesh component
	//tempEnt = ecs.create();
	//Transform newModelXForm{};
	//newModelXForm.SetPos(glm::vec3{ 0.f, 45.f, -30.f });
	//newModelXForm.SetScale(glm::vec3{ 5.f, 5.f, 5.f });
	//ecs.emplace<Transform>(tempEnt, newModelXForm);
	//MeshRenderer sphereMeshComp{};
	//sphereMeshComp.SetModel("ogre");
	//sphereMeshComp.SetMaterial(0, "PBR");
	//ecs.emplace<MeshRenderer>(tempEnt, sphereMeshComp);
}

void EntityManager::LateLoad()
{
}

void EntityManager::Init()
{
}

void EntityManager::Update()
{
}

void EntityManager::FixedUpdate()
{
}

void EntityManager::LateUpdate()
{
}

void EntityManager::Exit()
{
}

void EntityManager::Unload()
{
}

entt::registry& EntityManager::GetECS(void)
{
    return ecs;
}
