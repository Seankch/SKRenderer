/*****************************************************
    Includes
*****************************************************/
#include <Graphics/ObjectPicker.h>
#include <Managers/GraphicsSystem.h>
#include <Managers/WindowsManager.h>
#include <Components/Image.h>
#include <string>
#include <iostream>

// ImGui
#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#pragma warning(pop)

ObjectPicker::ObjectPicker()
{
}

void ObjectPicker::Init(GraphicsSystem::VIEWPORT_WINDOW _window, std::string const& _viewportName)
{
    // Create framebuffer for object picker
    mViewportNameMap.emplace(_window, _viewportName);
    mFBONameMap.emplace(_viewportName, "ObjectPicker" + _viewportName);
    RendererManager::GetInstance()->CreateFrameBuffer(mFBONameMap[_viewportName], false);
}

void ObjectPicker::WriteToFBO(GraphicsSystem::VIEWPORT_WINDOW _window)
{
    // Update to newest size
    ImGui::Begin(mViewportNameMap[_window].c_str());
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    RendererManager::GetInstance()->RescaleFrameBuffer(mFBONameMap[mViewportNameMap[_window]], (int)windowSize.x, (int)windowSize.y, (_window == GraphicsSystem::VW_GAME), false);
    ImGui::End();

    // Write to object picker FBO
    RendererManager::GetInstance()->BeginRender(mFBONameMap[mViewportNameMap[_window]]);

    // Get object picker shader
    Shader* shader = GraphicsSystem::GetInstance()->mShaders["ObjectPicker"];
    if (!shader)
        return;

    // Get camera matrices
    Camera* cam = GraphicsSystem::GetInstance()->GetCamera(_window);
    glm::mat4 V = cam->GetViewXForm();
    glm::mat4 P = cam->GetProjXForm();

    // Render all game objects onto FBO
    auto& reg = EntityManager::GetInstance()->mECS;
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has mesh renderer component
        if (!reg.all_of<Transform, MeshRenderer>(ent)) {
            // Entity does not have mesh renderer, skip
            continue;
        }

        // Entity has transform & mesh renderer component, begin render
        Transform& xform = reg.get<Transform>(ent);
        MeshRenderer& mr = reg.get<MeshRenderer>(ent);

        // Skip skybox
        if (reg.get<Name>(ent).GetName().find("Skybox") != std::string::npos)
            continue;

        // Check if material is rendering
        if (!mr.IsRendering()) {
            // If not rendering, skip
            continue;
        }

        // Compute model transform matrix
        glm::vec3 rotRadians = glm::radians(xform.GetRot());
        glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
        glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
        glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
        glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
        glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

        // Prepare variables for render
        Model* model = ResourceManager::GetInstance()->GetResource<Model>(GraphicsSystem::GetInstance()->mModels[mr.GetModel()]);
        if (!model)
        {
            DebugLogger::GetInstance()->DebugLog(DebugLogger::DMSG_ERROR, "Model does not exist!");
            continue;
        }

        // Pass entity ID to shader
        uint32_t id = static_cast<uint32_t>(ent) + 1;
        shader->SetUniform("UEntityIDR", ((id >> 16) & 0xFF));
        shader->SetUniform("UEntityIDG", ((id >> 8) & 0xFF));
        shader->SetUniform("UEntityIDB", (id & 0xFF));

        // Render GO
        for (size_t i = 0; i < model->mMeshes.size(); ++i) {
            RendererManager::GetInstance()->Render(model->mMeshes[i], *shader, glm::vec4{}, M * model->mMeshes[i].initialXFormMat, V, P);
        }
    }

    // End of writing
    RendererManager::GetInstance()->EndRender();
}

void ObjectPicker::WriteToFBOImage(GraphicsSystem::VIEWPORT_WINDOW _window)
{
    // Update to newest size
    if (GraphicsSystem::GetInstance()->GetIsMaximized())
    {
        // Rescale framebuffer based on game window
        int width = WindowsManager::GetInstance()->mDrawableWidth;
        int height = WindowsManager::GetInstance()->mDrawableHeight;
        RendererManager::GetInstance()->RescaleFrameBuffer(mFBONameMap[mViewportNameMap[_window]], width, height, (_window == GraphicsSystem::VW_GAME), false);
    }
    else
    {
        // Rescale framebuffer based on imgui window
        ImGui::Begin(mViewportNameMap[_window].c_str());
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        RendererManager::GetInstance()->RescaleFrameBuffer(mFBONameMap[mViewportNameMap[_window]], (int)windowSize.x, (int)windowSize.y, (_window == GraphicsSystem::VW_GAME), false);
        ImGui::End();
    }

    // Write to object picker FBO
    RendererManager::GetInstance()->BeginRender(mFBONameMap[mViewportNameMap[_window]]);

    // Get object picker shader
    Shader* shader = GraphicsSystem::GetInstance()->mShaders["ObjectPicker"];
    if (!shader)
        return;

    // Get camera matrices
    Camera* cam = GraphicsSystem::GetInstance()->GetCamera(_window);
    glm::mat4 defaultViewXForm = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1, 1 };
    glm::mat4 P = cam->GetProjXForm();

    // Render all game objects onto FBO
    auto& reg = EntityManager::GetInstance()->mECS;
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has image component
        if (!reg.all_of<Transform, Image>(ent)) {
            // Entity does not have image, skip
            continue;
        }

        // Entity has transform & image component, begin render
        Transform& xform = reg.get<Transform>(ent);
        Image& img = reg.get<Image>(ent);

        // Check if Image is rendering
        if (!img.IsRendering()) {
            // If not rendering, skip
            continue;
        }

        // Compute model transform matrix
        glm::vec3 rotRadians = glm::radians(xform.GetRot());
        glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
        glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
        glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
        glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
        glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

        // Prepare variables for render
        Model& model = *ResourceManager::GetInstance()->GetResource<Model>(GraphicsSystem::GetInstance()->mModels["Quad"]);

        // Pass entity ID to shader
        uint32_t id = static_cast<uint32_t>(ent) + 1;
        shader->SetUniform("UEntityIDR", ((id >> 16) & 0xFF));
        shader->SetUniform("UEntityIDG", ((id >> 8) & 0xFF));
        shader->SetUniform("UEntityIDB", (id & 0xFF));

        // Render GO
        RendererManager::GetInstance()->Render(model.mMeshes.front(), *shader, glm::vec4{}, M, defaultViewXForm, P);
    }

    // End of writing
    RendererManager::GetInstance()->EndRender();
}

uint32_t ObjectPicker::ReadPixel(GraphicsSystem::VIEWPORT_WINDOW _window)
{
    glm::vec2 openglMousePos{};
    if (GraphicsSystem::GetInstance()->GetIsMaximized())
    {
        openglMousePos = InputManager::GetInstance()->GetMousePos();
    }
    else
    {
        ImGui::Begin(GraphicsSystem::GetInstance()->GetFrameBufferName(_window).c_str());

        // Compute ImGui mouse pos
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImVec2 minWindowPos = { ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x, ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y };
        ImVec2 maxWindowPos = { minWindowPos.x + size.x, minWindowPos.y + size.y };
        glm::vec2 imguiMousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };

        // Convert ImGui mouse pos to screen coords
        openglMousePos.x = imguiMousePos.x - minWindowPos.x;
        openglMousePos.y = size.y - (imguiMousePos.y - minWindowPos.y);
        ImGui::End();
    }

    // Read pixel from FBO
    uint32_t id{};
    id = RendererManager::GetInstance()->ReadPixel(mFBONameMap[mViewportNameMap[_window]], static_cast<int>(openglMousePos.x), static_cast<int>(openglMousePos.y));
    
    // Return entity ID
    return (id == 0)? entt::null : (id - 1);
}

void ObjectPicker::FreeObjectPickerFBO(GraphicsSystem::VIEWPORT_WINDOW _window)
{
    // Free FBO associated with viewport's object picker
    RendererManager::GetInstance()->DeleteFrameBuffer(mFBONameMap[mViewportNameMap[_window]]);
}

bool ObjectPicker::IsUIHovered(entt::entity _id)
{
    // For object picking with mouse
    if (GraphicsSystem::GetInstance()->IsFrameBufferExist(GraphicsSystem::VW_GAME))
    {
        // Read pixel from game viewport
        entt::entity selectedEntity = static_cast<entt::entity>(ObjectPicker::GetInstance()->ReadPixel(GraphicsSystem::VW_GAME));
        if (selectedEntity == _id)
        {
            DebugLogger::GetInstance()->DebugLog(DebugLogger::DMSG_COUT, "UI Hovered");
            return true;
        }
    }

    // Not hovered, return false
    return false;
}

bool ObjectPicker::RaySphereIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter, float sphereRadius) {
    // Compute ray sphere collision
    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float d = b * b - 4 * a * c;
    return (d > 0);
}

bool ObjectPicker::IsMouseHoverEditor(Transform const& _transform)
{
    ImGui::Begin(GraphicsSystem::GetInstance()->GetFrameBufferName(GraphicsSystem::VW_EDITOR).c_str());

    // Compute ImGui mouse pos
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 minWindowPos = { ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x, ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y };
    ImVec2 maxWindowPos = { minWindowPos.x + size.x, minWindowPos.y + size.y };
    glm::vec2 imguiMousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };

    // Convert ImGui mouse pos to NDC coords
    glm::vec2 openglMousePos;
    openglMousePos.x = ((imguiMousePos.x - minWindowPos.x) / size.x) * 2.f - 1.f;
    openglMousePos.y = 1.f - ((imguiMousePos.y - minWindowPos.y) / size.y) * 2.f;

    // Compute clip coords
    glm::vec4 rayClip = glm::vec4(openglMousePos.x, openglMousePos.y, -1.f, 1.f);

    // Compute camera coords
    Camera* cam = GraphicsSystem::GetInstance()->GetCamera(GraphicsSystem::VW_EDITOR);
    glm::mat4 invViewProj = glm::inverse(cam->GetProjXForm());
    glm::vec4 rayEye = invViewProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.f, 0.f);

    // Convert to world coords
    glm::vec3 rayWorld = glm::normalize(glm::inverse(cam->GetViewXForm()) * rayEye);

    // Compute ray intersection from camera to object
    glm::vec3 rayOrigin = cam->GetPosition();
    if (RaySphereIntersect(rayOrigin, rayWorld, _transform.GetPos(), _transform.GetScale().x)) {
        // Intersected, return true
        ImGui::End();
        return true;
    }

    // No intersection, return false
    ImGui::End();
    return false;
}
