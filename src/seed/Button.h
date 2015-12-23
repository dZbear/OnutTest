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

        
	private:

        string      m_cmd;
        Sprite*     m_sprite;
	};
}