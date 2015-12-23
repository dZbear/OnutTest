#include "Button.h"
#include "Sprite.h"

namespace seed
{
    Button::Button()
        : m_sprite(nullptr)
    {
    }

    Button::~Button()
    {

    }

    void Button::SetCmd(const string& in_cmd)
    {
        m_cmd = in_cmd;
    }

    string& Button::GetCmd()
    {
        return m_cmd;
    }

    void Button::SetSprite(Sprite* in_sprite)
    {
        m_sprite = in_sprite;
    }

    Sprite* Button::GetSprite()
    {
        return m_sprite;
    }
}





