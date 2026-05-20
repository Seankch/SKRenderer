#ifndef LIGHT_H
#define LIGHT_H

/*****************************************************
    Includes
*****************************************************/
#include <glm/glm.hpp>

class Light
{
public:
    enum LIGHT_TYPE 
    {
        LT_DIRECTIONAL, // Directional light
        LT_POINT,       // Point light
    };

    Light();
    ~Light();

    // Getters/Setters
    glm::vec3 GetLightColor(void) const;
    void SetLightColor(glm::vec3 _color);
    LIGHT_TYPE GetLightType(void) const;
    void SetLightType(LIGHT_TYPE _type);
    float GetIntensity(void) const;
    void SetIntensity(float _intensity);
    glm::vec3 GetDirection(void) const;
    void SetDirection(glm::vec3 _dir);
    float GetRange(void) const;
    void SetRange(float _range);
private:
    glm::vec3 mColor; // RGB, no alpha
    LIGHT_TYPE mLightType;
    float mIntensity;
    glm::vec3 mDirection; // For directional light
    float mRange; // For point light
};

#endif
