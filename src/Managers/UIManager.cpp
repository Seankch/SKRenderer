/*****************************************************
    Includes
*****************************************************/
#include "Managers/UIManager.h"
#include "Managers/GraphicsSystem.h"
#include "Components/Transform.h"
#include "Managers/WindowsManager.h"
#include "Graphics/ObjectPicker.h"

//UI Buttons
#include "UI/Button.h"

void UIManager::Init()
{
}

void UIManager::Load()
{
}

void UIManager::Update()
{
    //Hide all menus (Mainly for changing of scenes)
    if (mHideAllMenus)
    {
        // Disable distortion for UI
        GraphicsSystem::GetInstance()->mShaders["ScreenShader"]->SetUniform("UDistortAmount", 1.0f);

        auto& reg = EntityManager::GetInstance()->mECS;
        for (auto& ent : reg.view<Button>())
        {
            if (auto* imageComp = EntityManager::GetInstance()->mECS.try_get<Image>(ent))
                imageComp->SetIsRendering(false);
        }
    }
    else
    {
        // Enable distortion
        GraphicsSystem::GetInstance()->mShaders["ScreenShader"]->SetUniform("UDistortAmount", 0.0f);
    }

    if (CoreEngine::GetInstance()->mGameState != CoreEngine::GameState::GS_STOP)
    {
        int maxUiIndex = 0;

        //Update Button Lerping
        auto& reg = EntityManager::GetInstance()->mECS;
        for (auto& ent : reg.view<Button>())
        {
            auto& button = reg.get<Button>(ent);

            if (maxUiIndex < button.GetUiSequenceNum())
                maxUiIndex = button.GetUiSequenceNum();

            Image& imageComp = reg.get<Image>(ent);
            if (imageComp.IsRendering() == true && mCurrUiIndex == button.GetUiSequenceNum())
            {
                button.OnHover(WindowsManager::GetInstance()->mDeltaTime);
                PlayMenuSoundOnce("UI_Hover_new.wav");

                //if (InputManager::GetInstance()->GetMouseButtonDown(KeyCode::MOUSE_BUTTON_1))
                if (InputManager::GetInstance()->GetKeyDown(KeyCode::ENTER))
                {
                    button.OnClick();
                    PlayMenuSoundOnce("UI_Select_New.wav");
                }
            }
            else
                button.OffHover(WindowsManager::GetInstance()->mDeltaTime);
        }

        //Keyboard controls for menus
        if (mHideAllMenus == false)
        {
            if (InputManager::GetInstance()->GetKeyDown(KeyCode::S) || InputManager::GetInstance()->GetKeyDown(KeyCode::DOWN))
            {
                mCurrUiIndex++;
                if (mCurrUiIndex > maxUiIndex)
                    mCurrUiIndex = -1;

                mSoundPlayed = false;
            }
            if (InputManager::GetInstance()->GetKeyDown(KeyCode::W) || InputManager::GetInstance()->GetKeyDown(KeyCode::UP))
            {
                mCurrUiIndex--;
                if (mCurrUiIndex < -1)
                    mCurrUiIndex = maxUiIndex;

                mSoundPlayed = false;
            }
        }
    }
}

void UIManager::FixedUpdate()
{
}

void UIManager::Free()
{
}

void UIManager::Unload()
{
    Destroy();
}

glm::vec3 UIManager::LerpButton(float _timer, float _duration, glm::vec3 _start, glm::vec3 _end)
{
    float x = (_timer / _duration);
    return (-(cos(glm::pi<float>() * x) - 1) / 2) * (_end - _start) + _start;
}

void UIManager::Render(glm::mat4 const& _projXForm)
{
    // If UI is not rendering, exit
    if (!mIsRenderUI)
        return;

    // Get default view xform
    glm::mat4 defaultViewXForm = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1, 1 };

    // Store to temp list
    std::vector<entt::entity> uiRenderList;
    auto& reg = EntityManager::GetInstance()->mECS;
    for (auto& ent : reg.view<entt::entity>())
    {
        // Check if entity has Image/transform component
        if (!reg.all_of<Transform, Image>(ent)) {
            // Entity does not have Image/transform, skip
            continue;
        }

        // Check if material is rendering
        Image& img = reg.get<Image>(ent);
        if (!img.IsRendering()) {
            // If not rendering, skip
            continue;
        }

        // Add to list
        uiRenderList.emplace_back(ent);
    }

    // Sort list based on z-position
    std::sort(uiRenderList.begin(), uiRenderList.end(), [&](entt::entity _left, entt::entity _right) 
    {
        const auto& leftXForm = reg.get<Transform>(_left);
        const auto& rightXForm = reg.get<Transform>(_right);
        return leftXForm.GetPos().z < rightXForm.GetPos().z; // Lower value rendered first
    });

    // Render UI elements
    for (auto& ent : uiRenderList)
    {
        // Entity has transform & image component, begin render
        Transform& xform = reg.get<Transform>(ent);
        Image& img = reg.get<Image>(ent);

        // Compute model transform matrix
        glm::vec3 rotRadians = glm::radians(xform.GetRot());
        glm::mat4 translateMat = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xform.GetPos().x, xform.GetPos().y, xform.GetPos().z, 1 };
        glm::mat4 rotMatZ = { cosf(rotRadians.z), sinf(rotRadians.z), 0, 0, -sinf(rotRadians.z), cosf(rotRadians.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        glm::mat4 rotMatY = { cosf(rotRadians.y), 0, -sinf(rotRadians.y), 0, 0, 1, 0, 0, sinf(rotRadians.y), 0, cosf(rotRadians.y), 0, 0, 0, 0, 1 };
        glm::mat4 rotMatX = { 1, 0, 0, 0, 0, cosf(rotRadians.x), sinf(rotRadians.x), 0, 0, -sinf(rotRadians.x), cosf(rotRadians.x), 0, 0, 0, 0, 1 };
        glm::mat4 scaleMat = { xform.GetScale().x, 0, 0, 0, 0, xform.GetScale().y, 0, 0, 0, 0, xform.GetScale().z, 0, 0, 0, 0, 1 };
        glm::mat4 M = translateMat * rotMatZ * rotMatY * rotMatX * scaleMat;

        // Prepare variables for render
        glm::vec4 tintColor = img.GetColor();
        GUID& quadGuid = GraphicsSystem::GetInstance()->mModels["Quad"];
        Model& model = *ResourceManager::GetInstance()->GetResource<Model>(quadGuid);
        Shader& shader = *(GraphicsSystem::GetInstance()->mShaders[img.GetShader()]);
        Texture* tex = ResourceManager::GetInstance()->GetResource<Texture>(GraphicsSystem::GetInstance()->mTextures[img.GetTexture()]);
        if (!tex) {
            // UI element does not have a texture, don't render
            continue;
        }

        // Pass uniforms to shader
        shader.SetUniformTexture("UMainTex", tex->textureID);
        shader.SetUniform("UHasPremultipliedAlpha", (img.HasPremultipliedAlpha())? GL_TRUE : GL_FALSE);

        // Render UI element
        RendererManager::GetInstance()->Render(model.mMeshes.front(), shader, tintColor, M, defaultViewXForm, _projXForm, img.HasPremultipliedAlpha());
    }
}

UIManager::UIManager() : mIsRenderUI{ true }
{
}

void UIManager::SetMenu(std::string _targetMenu, bool _openMenu)
{
    if (_openMenu)
    {
        mHideAllMenus = false;
        mCurrUiIndex = -1;
    }

    auto& reg = EntityManager::GetInstance()->mECS;
    for (auto& ent : reg.view<Button>())
    {
        //Enable UI targeting the specific menu
        Button& buttonComp = reg.get<Button>(ent);
        if (buttonComp.GetMenuType() == _targetMenu)
        {
            if (auto* imageComp = EntityManager::GetInstance()->mECS.try_get<Image>(ent))
                imageComp->SetIsRendering(_openMenu);
        }
    }
}


void UIManager::PlayMenuSoundOnce(std::string _targetSoundFile)
{
    if (mSoundPlayed == false)
    {
        mSoundPlayed = true;
        AudioManager::GetInstance()->PlaySounds(_targetSoundFile, glm::vec3(0.f), 0.f);
    }
}