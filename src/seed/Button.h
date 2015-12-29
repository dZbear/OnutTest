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

        vector<int>&    GetEnabledGamepads() { return m_gamePads; }
        void            SetEnabledGamepads(vector<int>& in_gamepads);

        bool    CanBeFocused(int in_playerIndex = 1);
        
    private:

        string      m_cmd;
        Sprite*     m_sprite;
        bool        m_pressed;
        vector<int> m_gamePads; // index of gamepads that can focus this Button, if empty, only player 1 can focus
    };
}