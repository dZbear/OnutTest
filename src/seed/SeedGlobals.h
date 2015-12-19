#pragma once

#include <vector>
#include <unordered_map>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
using namespace std;

namespace seed
{
	class Sprite;
	class View;

	typedef unordered_map<string, Sprite*>	SpriteMap;
	typedef vector<Sprite*>					SpriteVect;
	typedef unordered_map<string, View*>	ViewMap;
	typedef vector<View*>					ViewStack;
}