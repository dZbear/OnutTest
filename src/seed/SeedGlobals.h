#pragma once

#include <vector>
#include <unordered_map>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
using namespace std;

namespace seed
{
	class Sprite;
    class Button;
	class View;
    class Node;

	typedef unordered_map<string, Sprite*>	SpriteMap;
	typedef vector<Sprite*>					SpriteVect;
    typedef vector<Node*>					NodeVect;
	typedef unordered_map<string, View*>	ViewMap;
	typedef vector<View*>					ViewStack;
    typedef vector<Button*>					ButtonVect;


    enum class eAppCommand
    {
        PUSH_VIEW,
        POP_VIEW,
        SWITCH_VIEW,
        REMOVE_VIEW
    };

    struct SCommand
    {
        eAppCommand     m_command;
        vector<string>  m_params;
    };
}