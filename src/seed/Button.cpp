#include "Button.h"
#include "Sprite.h"

namespace seed
{
    Button::Button()
        : m_sprite(nullptr)
        , m_pressed(false)
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

    void Button::SetEnabledGamepads(vector<int>& in_gamepads)
    {
        m_gamePads.clear();
        m_gamePads = in_gamepads;
    }

    bool Button::CanBeFocused(int in_playerIndex)
    {
        if (in_playerIndex == 1 && m_gamePads.size() == 0)
        {
            return true;
        }

        for (int i : m_gamePads)
        {
            if (i == in_playerIndex)
            {
                return true;
            }
        }
        return false;
    }
}





