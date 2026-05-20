#ifndef TEXT_MANAGER_H
#define TEXT_MANAGER_H

/*****************************************************
    Includes
*****************************************************/
#include <vector>
#include <Graphics/GraphicsDefine.h>
#include <Graphics/Shader.h>
#include <Components/Text.h>
#include <Components/Transform.h>

class TextManager
{
public:
    //Manager Functions
    void Init();
    void Load();
    void Update();
    void FixedUpdate();
    void Free();
    void Unload();

    // Enum for text appearing mode
    enum TextMode
    {
        TM_GAME_ONLY,
        TM_EDITOR_ONLY,
        TM_ALL
    };

    // Function to draw text on screen
    void Render(glm::mat4 const& _viewXForm, glm::mat4 const& _projXForm);
private:
    //For singleton
    TextManager();
    friend Singleton<TextManager>;

    // Variable for default font shader
    Shader* mDefaultFontShader{};
};

#endif
