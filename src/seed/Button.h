#pragma once
#include "SeedGlobals.h"
#include "onut.h"

namespace seed
{
    class Sprite;
    class Button
    {
    public:

        Button();
        virtual ~Button();

        void    SetCmd(const string& in_cmd);
        string& GetCmd();

        void    SetSprite(Sprite* in_sprite);
        Sprite* GetSprite();

        void    SetPressed(bool in_pressed) { m_pressed = in_pressed; }
        bool    IsPressed() { return m_pressed; }

        
    private:

        string      m_cmd;
        Sprite*     m_sprite;
        bool        m_pressed;
    };
}