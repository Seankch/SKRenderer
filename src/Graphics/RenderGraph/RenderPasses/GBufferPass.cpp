#include "Graphics/RenderGraph/RenderPasses/GBufferPass.h"
#include "Components/MeshRenderer.h"
#include "Components/Transform.h"

void GBufferPass::Execute(RenderContext const& _renderContext)
{
    // Get main renderer
    RendererBase* mainRenderer = _renderContext.rendererMgr.GetRenderer();

    // Bind and clear frame buffer
    _renderContext.rendererMgr.BeginRender(_renderContext.fboIdx, _renderContext.V, _renderContext.P);

    // Render all game objects
    auto& reg = _renderContext.entityMgr.GetECS();
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!reg.all_of<Transform, MeshRenderer>(ent))
        {
            // Entity does not have MeshRenderer, skip
            continue;
        }

        // Entity has transform & MeshRenderer component, begin render
        Transform& xform = reg.get<Transform>(ent);
        MeshRenderer& mr = reg.get<MeshRenderer>(ent);
        // Check if mesh is rendering
        if (!mr.IsRendering())
        {
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
        Model* model = _renderContext.resourceMgr.GetResource<Model>(mr.GetModel());
        if (!model)
            continue;

        // Render GO
        for (size_t j = 0; j < model->mMeshes.size(); ++j)
        {
            // Get mesh's material instance
            Material* matInstance = mr.GetMatInstance(static_cast<int>(j));
            if (!matInstance || matInstance->GetName().empty())
            {
                // Fall back to default material
                matInstance = _renderContext.resourceMgr.GetResource<Material>(mr.GetMaterialList()[j]);

                // If default material does not exists, don't render
                if (!matInstance || matInstance->GetName().empty())
                {
                    continue;
                }
            }

            // Check if material has a valid shader
            if (matInstance->GetShader().empty())
            {
                continue;
            }

            // Set base uniform variables
            Shader& shader = *(_renderContext.resourceMgr.GetResource<Shader>(matInstance->GetShader()));

            // Load other uniforms to shader
            mainRenderer->LoadUniformsToShader(*matInstance, shader);

            // Render all meshes in model
            mainRenderer->SetCullMode(matInstance->GetCullMode());
            mainRenderer->Render(model->mMeshes[j], *matInstance, M * model->mMeshes[j].initialXFormMat, matInstance->GetIsPremultipliedAlpha());
            mainRenderer->ResetCullMode();
        }
    }

    // Unbind frame buffer
    _renderContext.rendererMgr.EndRender();
}
