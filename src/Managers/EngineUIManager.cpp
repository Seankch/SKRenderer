#include "Managers/WindowsManager.h"
#include "Managers/EngineUIManager.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "Components/Light.h"

#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <ImGuizmo.h>

EngineUIManager::EngineUIManager(EntityManager* _em, RendererManager* _rm, LightingSystem* _ls)
    : entityMgr{ _em }, rendererMgr{ _rm }, lightingSystem{ _ls }
{
}

EngineUIManager::~EngineUIManager()
{
}

void EngineUIManager::Load()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_DockingEmptyBg].w = 0.0f;
	}

	// Setup Platform/Renderer backends
	// Init for Vulkan
	VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(rendererMgr->GetRenderer());
	renderer->InitImGUI();
}

void EngineUIManager::LateLoad()
{
}

void EngineUIManager::Init()
{
}

void EngineUIManager::Update()
{
    // Begin imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Set window host flags
    ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBackground;

    // Transparent fullscreen dockspace host window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Set dockspace
    ImGuiID dockspaceID = ImGui::GetID("Dockspace");
    ImGui::Begin("##DockspaceHost", nullptr, hostFlags);
    ImGui::PopStyleVar(2);
    ImGui::DockSpace(dockspaceID, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // Render engine UI
    // Render main panel
    MainPanel();
    GraphicsPanel();
}

void EngineUIManager::FixedUpdate()
{
}

void EngineUIManager::LateUpdate()
{
}

void EngineUIManager::Exit()
{
}

void EngineUIManager::Unload()
{
    // Wait device idle before destroying imgui
    rendererMgr->WaitDeviceIdle();

	// Store configurations to ini file
	ImGuiContext* g = ImGui::GetCurrentContext();
	std::string filename("../imgui.ini");
	g->IO.IniFilename = filename.c_str();
	if (g->SettingsLoaded && g->IO.IniFilename != NULL)
		ImGui::SaveIniSettingsToDisk(g->IO.IniFilename);

	// Shutdown ImGui
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void EngineUIManager::Render()
{
	ImGui::Render();
}

void EngineUIManager::PostRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(context);
	}
}

void EngineUIManager::MainPanel()
{
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 panelSize{ 420.0f, 425.0f };
    const ImVec2 panelPos{ io.DisplaySize.x - panelSize.x - 10.0f, 10.0f };

    ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

    if (ImGui::Begin("Main Panel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        ImGui::BeginChild("Object Window", ImVec2(0, 0), true);

        // FPS display
        float fps = ImGui::GetIO().Framerate;
        float ms = 1000.0f / fps;
        ImGui::Text("Current FPS: %.1f", fps, ms);

        ImGui::Separator();

        auto& reg = entityMgr->GetECS();
        auto view = reg.view<Transform>();

        std::vector<entt::entity> entities;
        entities.reserve(view.size());
        for (auto e : view)
            entities.push_back(e);

        if (entities.empty())
        {
            ImGui::TextDisabled("No entities with Transform.");
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        auto isInList = [&](entt::entity e)
        {
            return std::find(entities.begin(), entities.end(), e) != entities.end();
        };

        if (mSelected == entt::null || !isInList(mSelected))
        {
            mSelectedIndex = 0;
            mSelected = entities[0];
        }

        auto GetEntityLabel = [&](entt::entity e) -> std::string
        {
            uint32_t id = (uint32_t)entt::to_integral(e);
            return "Entity " + std::to_string(id) + "##" + std::to_string(id);
        };

        uint32_t selId = (uint32_t)entt::to_integral(mSelected);
        std::string preview = "Entity " + std::to_string(selId);

        if (ImGui::BeginCombo("Object", preview.c_str()))
        {
            for (int i = 0; i < (int)entities.size(); ++i)
            {
                entt::entity e = entities[i];
                bool selected = (e == mSelected);

                std::string label = GetEntityLabel(e);
                if (ImGui::Selectable(label.c_str(), selected))
                {
                    mSelected = e;
                    mSelectedIndex = i;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Transform
        if (reg.all_of<Transform>(mSelected))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Transform& tr = reg.get<Transform>(mSelected);

                glm::vec3 pos = tr.GetPos();
                glm::vec3 rot = tr.GetRot();
                glm::vec3 scale = tr.GetScale();

                ImGui::PushItemWidth(-FLT_MIN);

                ImGui::Text("Position");
                if (ImGui::DragFloat3("##pos", &pos.x, 0.05f))
                    tr.SetPos(pos);

                ImGui::Text("Rotation");
                if (ImGui::DragFloat3("##rot", &rot.x, 0.5f))
                    tr.SetRot(rot);

                ImGui::Text("Scale");
                if (ImGui::DragFloat3("##scale", &scale.x, 0.05f, 0.01f, 100.0f))
                    tr.SetScale(scale);

                ImGui::PopItemWidth();
            }
        }

        // Light
        if (reg.all_of<Light>(mSelected))
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Get light component
                Light& lightComp = reg.get<Light>(mSelected);

                // Set light type
                char const* lightTypes[] = { "Directional", "Point" };
                int type = static_cast<int>(lightComp.GetLightType());
                if (ImGui::Combo("Light Type", &type, lightTypes, IM_ARRAYSIZE(lightTypes)))
                {
                    lightComp.SetLightType(static_cast<Light::LIGHT_TYPE>(type));
                }

                ImGui::Separator();

                // Set color
                glm::vec3 color = lightComp.GetLightColor();
                if (ImGui::ColorEdit3("Color", &color.x))
                {
                    lightComp.SetLightColor(color);
                }

                // Set intensity
                float intensity = lightComp.GetIntensity();
                if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f))
                {
                    lightComp.SetIntensity(intensity);
                }

                // Set direction or range based on light type
                if (lightComp.GetLightType() == Light::LT_DIRECTIONAL)
                {
                    glm::vec3 direction = lightComp.GetDirection();
                    if (ImGui::DragFloat3("Direction", &direction.x, 0.05f))
                    {
                        lightComp.SetDirection(direction);
                    }
                }
                else if (lightComp.GetLightType() == Light::LT_POINT)
                {
                    float range = lightComp.GetRange();
                    if (ImGui::DragFloat("Range", &range, 0.5f, 0.01f, 1000.0f))
                    {
                        lightComp.SetRange(range);
                    }
                }
            }
        }

        // Mesh Renderer
        if (reg.all_of<MeshRenderer>(mSelected))
        {
            if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                MeshRenderer& mr = reg.get<MeshRenderer>(mSelected);

                // Display model name
                ImGui::Text("Model Name: %s", mr.GetModel().c_str());
                
                // Set is rendering
                bool isRendering = mr.IsRendering();
                ImGui::Checkbox("Is Rendering", &isRendering);
                mr.SetIsRendering(isRendering);

                // Get Material instance list
                std::vector<Material>& matInstances = mr.GetMatInstanceList();
                std::vector<std::string>& matList = mr.GetMaterialList();

                if (!matInstances.empty())
                {
                    int& selectedMatIndex = mSelectedMatInstance[mSelected];

                    if (selectedMatIndex < 0 || selectedMatIndex >= (int)matInstances.size())
                        selectedMatIndex = 0;

                    ImGui::Separator();
                    ImGui::Text("Material Instances");

                    std::string selectedMatLabel;
                    if (selectedMatIndex < (int)matList.size() && !matList[selectedMatIndex].empty())
                        selectedMatLabel = "Submesh " + std::to_string(selectedMatIndex) + " : " + matList[selectedMatIndex];
                    else
                        selectedMatLabel = "Submesh " + std::to_string(selectedMatIndex);

                    if (ImGui::BeginCombo("Material Instance", selectedMatLabel.c_str()))
                    {
                        for (int i = 0; i < (int)matInstances.size(); ++i)
                        {
                            std::string matLabel;
                            if (i < (int)matList.size() && !matList.empty())
                                matLabel = "Submesh " + std::to_string(i) + " : " + matList[i];
                            else
                                matLabel = "Submesh " + std::to_string(i);

                            bool isSelected = (selectedMatIndex == i);
                            if (ImGui::Selectable(matLabel.c_str(), isSelected))
                                selectedMatIndex = i;

                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    // =========================
                    // Material Inspector
                    // =========================
                    if (selectedMatIndex >= 0 && selectedMatIndex < (int)matInstances.size())
                    {
                        Material& mat = matInstances[selectedMatIndex];

                        ImGui::Separator();

                        ImGui::Text("Material: %s", mat.GetName().c_str());
                        ImGui::Text("Shader: %s", mat.GetShader().c_str());

                        ImGui::Separator();
                        ImGui::Text("Uniforms");

                        auto& uniforms = mat.GetUniformMap();

                        for (auto& [name, value] : uniforms)
                        {
                            std::visit([&](auto& v)
                                {
                                    using T = std::decay_t<decltype(v)>;

                                    if constexpr (std::is_same_v<T, glm::vec2>)
                                    {
                                        ImGui::DragFloat2(name.c_str(), &v.x, 0.05f);
                                    }
                                    else if constexpr (std::is_same_v<T, glm::vec3>)
                                    {
                                        ImGui::DragFloat3(name.c_str(), &v.x, 0.05f);
                                    }
                                    else if constexpr (std::is_same_v<T, glm::vec4>)
                                    {
                                        ImGui::ColorEdit4(name.c_str(), &v.x);
                                    }
                                    else if constexpr (std::is_same_v<T, float>)
                                    {
                                        ImGui::DragFloat(name.c_str(), &v, 0.05f);
                                    }
                                    else if constexpr (std::is_same_v<T, int>)
                                    {
                                        ImGui::InputInt(name.c_str(), &v);
                                    }
                                    else if constexpr (std::is_same_v<T, unsigned>)
                                    {
                                        int temp = static_cast<int>(v);
                                        if (ImGui::InputInt(name.c_str(), &temp))
                                        {
                                            if (temp < 0) temp = 0; // prevent negative
                                            v = static_cast<unsigned>(temp);
                                        }
                                    }
                                    else if constexpr (std::is_same_v<T, unsigned>)
                                    {
                                        int temp = (int)v;
                                        if (ImGui::DragInt(name.c_str(), &temp, 1.0f, 0))
                                            v = (unsigned)temp;
                                    }
                                    else if constexpr (std::is_same_v<T, TextureEntry>)
                                    {
                                        ImGui::Text("%s", name.c_str());
                                        ImGui::BulletText("Texture: %s", v.mTextureName.c_str());
                                        ImGui::BulletText("ID: %d", v.ID);
                                    }

                                }, value);
                        }
                    }
                }
                else
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("No material instances.");
                }
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

void EngineUIManager::GraphicsPanel(void)
{
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 panelSize{ 300.0f, 150.0f };
    const ImVec2 panelPos{ 10.0f, 10.0f };

    ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

    if (ImGui::Begin("Graphics Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        ImGui::BeginChild("GraphicsChild", ImVec2(0, 0), true);

        // Ambient light settings
        if (ImGui::CollapsingHeader("Ambient Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Get current values
            float intensity = lightingSystem->GetAmbientIntensity();
            glm::vec3 color = lightingSystem->GetAmbientColor();

            // Intensity
            if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f))
            {
                lightingSystem->SetAmbientIntensity(intensity);
            }

            // Color
            if (ImGui::ColorEdit3("Color", &color.x))
            {
                lightingSystem->SetAmbientColor(color);
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}
