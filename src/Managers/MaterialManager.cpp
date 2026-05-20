#include "Managers/MaterialManager.h"
#include "Managers/GraphicsSystem.h"
#include "Managers/JSONManager.h"

MaterialManager::MaterialManager(ResourceManager* _resourceMgr, EntityManager* _entityMgr, JSONManager* _jsonMgr)
: resourceMgr{ _resourceMgr }, entityMgr{ _entityMgr }, jsonManager{ _jsonMgr }
{
}

MaterialManager::~MaterialManager()
{
}

void MaterialManager::LateLoad()
{
    AssignAllMaterialInstance();
}

void MaterialManager::Init()
{
}

void MaterialManager::Load()
{
}

void MaterialManager::Update()
{
}

void MaterialManager::FixedUpdate()
{
}

void MaterialManager::LateUpdate()
{
}

void MaterialManager::Exit()
{
}

void MaterialManager::Unload()
{
    ClearMaterialInstances();
}

void MaterialManager::CreateNewMaterial(std::string const& _matName)
{
    // If material already exists, don't create it
    if (resourceMgr->CheckResourceExist<Material>(_matName))
        return;

    // Create new material, set as PBR by default
    std::string defaultShaderName = "PBR";
    jsonManager->CreateNewSave();
    jsonManager->Save("MatName", _matName);
    jsonManager->Save("ShaderName", defaultShaderName);
    jsonManager->Save("TintColor", glm::vec4{1.f, 1.f, 1.f, 1.f});
    jsonManager->Save("CullMode", (int)Material::CM_CULL_NONE);
    jsonManager->Save("IsPremultipliedAlpha", false);

    // Fetch default shader properties (Default shader is PBR)
    Shader* shader = resourceMgr->GetResource<Shader>(defaultShaderName);
    if (!shader)
        return;
    std::vector<Uniform> const& uList = shader->GetUniformList();

    // Get material (new material will be default saved with PBR properties)
    Material* mat = resourceMgr->GetResource<Material>(defaultShaderName);
    if (!mat)
        return;

    // Save properties
    for (size_t i = 0; i < uList.size(); ++i)
    {
        Uniform const& u = uList[i];
        if (u.type == UniformType::UT_FLOAT_VEC2)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<glm::vec2>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT_VEC3)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<glm::vec3>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT_VEC4)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<glm::vec4>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<float>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_DOUBLE)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<double>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_INT)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<int>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_UNSIGNED_INT)
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<GLuint>(uList[i].uniformName).value());
        }
        else if ((u.type == UniformType::UT_TEXTURE_INDEX) || (u.type == UniformType::UT_SAMPLER2D))
        {
            jsonManager->Save(uList[i].uniformName, mat->GetUniform<TextureEntry>(uList[i].uniformName).value().mTextureName);
        }
    }

    // Save file once done
    jsonManager->SaveFile(_matName, "material", MATERIAL_DIR);
    jsonManager->DeleteNewSave();

    // Load created material
    resourceMgr->AddMaterial(_matName);
}

void MaterialManager::SaveMaterial(Material& _mat)
{
    // Save material object
    std::string matName = _mat.GetName();
    jsonManager->CreateNewSave();
    jsonManager->Save("MatName", matName);
    jsonManager->Save("ShaderName", _mat.GetShader());
    jsonManager->Save("CullMode", (int)_mat.GetCullMode());
    jsonManager->Save("IsPremultipliedAlpha", _mat.GetIsPremultipliedAlpha());

    // Fetch shader properties
    Shader* shader = resourceMgr->GetResource<Shader>(_mat.GetShader());
    if (!shader)
        return;
    std::vector<Uniform> const& uList = shader->GetUniformList();

    // Save properties
    for (size_t i = 0; i < uList.size(); ++i)
    {
        Uniform const& u = uList[i];
        if (u.type == UniformType::UT_FLOAT_VEC2)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<glm::vec2>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT_VEC3)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<glm::vec3>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT_VEC4)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<glm::vec4>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_FLOAT)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<float>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_DOUBLE)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<double>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_INT)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<int>(uList[i].uniformName).value());
        }
        else if (u.type == UniformType::UT_UNSIGNED_INT)
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<GLuint>(uList[i].uniformName).value());
        }
        else if ((u.type == UniformType::UT_TEXTURE_INDEX) || (u.type == UniformType::UT_SAMPLER2D))
        {
            jsonManager->Save(uList[i].uniformName, _mat.GetUniform<TextureEntry>(uList[i].uniformName).value().mTextureName);
        }
    }

    // Save material to file
    jsonManager->SaveFile(matName, "material", MATERIAL_DIR);
    jsonManager->DeleteNewSave();
}

void MaterialManager::AssignAllMaterialInstance(void)
{
    // Render all game objects
    auto& reg = entityMgr->GetECS();
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!reg.all_of<MeshRenderer>(ent)) 
        {
            // Entity does not have MeshRenderer, skip
            continue;
        }

        // Get mesh renderer
        MeshRenderer& mr = reg.get<MeshRenderer>(ent);

        // Get mesh renderer's model
        Model* mdl = resourceMgr->GetResource<Model>(mr.GetModel());
        if (!mdl)
            continue;

        // For each submesh, assign material instances
        for (size_t i = 0; i < mdl->mMeshes.size(); ++i) 
        {
            if (mr.GetMaterialList()[i].empty())
                continue;

            Material* mat = resourceMgr->GetResource<Material>(mr.GetMaterialList()[i]);            
            mr.AddMatInstance(*mat);
        }
    }
}

void MaterialManager::ClearMaterialInstances(void)
{
    // Render all game objects
    auto& reg = entityMgr->GetECS();
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has MeshRenderer component
        if (!reg.all_of<MeshRenderer>(ent)) {
            // Entity does not have MeshRenderer, skip
            continue;
        }

        // Get mesh renderer
        MeshRenderer& mr = reg.get<MeshRenderer>(ent);

        // Clear material instances
        mr.ClearMaterialList();
    }
}
