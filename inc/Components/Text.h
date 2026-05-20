#ifndef TEXT_H
#define TEXT_H

/*****************************************************
    Includes
*****************************************************/
#include <Graphics/Font.h>

// Text component
class Text
{
public:
    enum TEXT_STYLE
    {
        TS_CENTERED,
        TS_FIRST_CHAR_AT_POS
    };

    Text();
    ~Text();

    // Getters
    std::string const& GetTextString() const;
    std::string const& GetFontName() const;
    glm::vec4 const& GetTextColor() const;
    float GetTextSpacing() const;
    bool IsRendering() const;
    bool IsInGameText() const;
    TEXT_STYLE GetTextStyle() const;
    float GetNewLineOffset() const;
    bool GetIsTextFaceCamera() const;

    // Setters
    void SetTextString(std::string const& _textString);
    void SetFontName(std::string const& _fontName);
    void SetTextColor(glm::vec4 const& _textColor);
    void SetTextSpacing(float _textSpacing);
    void SetRendering(bool _isRendering);
    void SetIsInGameText(bool _isInGameText);
    void SetTextStyle(TEXT_STYLE _textStyle);
    void SetNewLineOffset(float _newLineOffset);
    void SetIsTextFaceCamera(bool _isFaceCamera);
private:
    // Variables
    std::string mTextString;
    std::string mFontName;
    glm::vec4 mTextColor;
    float mTextSpacing;
    bool mIsRendering;
    bool mIsInGameText;
    TEXT_STYLE mTextStyle;
    float mNewLineOffset;
    bool mIsFaceCamera;
};

#endif
